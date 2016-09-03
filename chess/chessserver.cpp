#include "chessserver.h"

#include <QTcpSocket>
#include <QDateTime>

constexpr int ChessServer::timeoutInterval;

ChessServer::ChessServer(QHostAddress address, quint16 port, QObject *parent)
    : QObject(parent), address(address), port(port)
{
    grantFunc = [] (QHostAddress, quint16) { return true; };
}

bool ChessServer::start()
{
    close();
    listener = new QTcpServer(this);
    if (!listener->listen(address, port))
    {
        return false;
    }
    connect(listener, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    startCheckAlive();
    return true;
}

void ChessServer::onNewConnection()
{
    QTcpSocket *sock = listener->nextPendingConnection();
    if (peer || !grantFunc(sock->peerAddress(), sock->peerPort()))
    {
        sock->close();
        delete sock;
    }
    else
    {
        if (peer)
        {
            sock->close();
            delete sock;
            return;
        }
        peer = new JsonSession(sock, this);
        connect(peer, SIGNAL(onMessage(JsonSession *, QJsonObject)), this, SLOT(onClientMessage(JsonSession *, QJsonObject)));
        connect(peer, SIGNAL(onHttpRequest(JsonSession*, QString)), this, SLOT(onHttpRequest(JsonSession*, QString)));

        startGame();
    }
}

void ChessServer::removeSession(JsonSession *sess, bool wait)
{
    if (!wait)
    {
        sess->close();
    }
    sess->deleteLater();
    sessions.remove(sess);
    if (sess == peer)
    {
        peer = nullptr;
    }
}

void ChessServer::onClientMessage(JsonSession *, QJsonObject obj)
{
    // TODO: multi-color support
    onMessage(Engine::nextColor(myColor), obj);
}

void ChessServer::command(const QJsonObject &obj)
{
    onMessage(myColor, obj);
}

void ChessServer::timerEvent(QTimerEvent *)
{
    qDebug() << "checking who is timed out...";
    QList<JsonSession *> sessionPointers = sessions.keys();
    for (JsonSession *sess : sessionPointers)
    {
        if (sess->lastActive().msecsTo(QDateTime::currentDateTimeUtc()) >= timeoutInterval)
        {
            removeSession(sess);
        }
    }
}

void ChessServer::onMessage(chess_t color, const QJsonObject &obj)
{
    QString type = obj["type"].toString();
    qDebug() << "server on message" << color << obj;
    if (type == "null")
    {
        // heartbeat
    }
    else if (type == "hello")
    {
        // nothing to do
    }
    else if (type == "place")
    {
        if (!isPlaying || turn != color)
        {
            // reject
            return;
        }
        // next turn
        turn = Engine::nextColor(turn);

        QJsonObject data = obj["data"].toObject();
        int row = data["row"].toInt(),
            col = data["col"].toInt();
        if (!place(color, row, col))
        {
            // reject
            return;
        }

        sendBoardBoth(row, col, true);

        // check win
        chess_t win = Engine::findWin(board);
        if (win != CH_SPACE)
        {
            isPlaying = false;
            sendWin(win);
        }
    }
    else if (type == "close")
    {
        isPlaying = false;
        if (color != myColor)
        {
            // peer disconnected
            peer->deleteLater();
            peer = nullptr;
            emit message(obj);
        }
        else
        {
            // ???
        }
    }
    else if (type == "error")
    {
        isPlaying = false;
        if (color != myColor)
        {
            // peer error
            peer->deleteLater();
            peer = nullptr;
            emit message(obj);
        }
        else
        {
            // self error ???
        }
    }
    else if (type == "new")
    {
        if (isPlaying || !peer)
        {
            // reject
            return;
        }
        startGame();
    }
}

void ChessServer::startGame()
{
    // "hello"s
    QJsonObject obj;
    obj["type"] = "hello";
    sendBoth(obj);

    // setup
    board = Engine::generate(15, 15);
    myColor = Engine::randomColor();

    // black first
    turn = CH_BLACK;
    isPlaying = true;

    sendColors();
    sendBoardBoth();
}

void ChessServer::close()
{
    stopCheckAlive();

    QList<JsonSession *> sessionPointers = sessions.keys();
    for (JsonSession *sess : sessionPointers)
    {
        removeSession(sess);
    }

    if (listener)
    {
        listener->close();
        listener->deleteLater();
        listener = nullptr;
    }

    if (peer)
    {
        peer->close();
        peer->deleteLater();
        peer = nullptr;
    }
}

void ChessServer::sendBoardBoth(int lastRow, int lastCol, bool inc)
{
    QJsonObject obj;
    obj["type"] = "update";
    QJsonObject data;
    data["row"] = lastRow;
    data["col"] = lastCol;
    data["turn"] = Engine::toJson(turn);
    if (!inc)
    {
        data["board"] = Engine::toJson(board);
    }
    obj["data"] = data;
    sendBoth(obj);
}

void ChessServer::sendWin(chess_t win)
{
    QJsonObject obj;
    obj["type"] = "win";
    obj["data"] = Engine::toJson(win);
    sendBoth(obj);
}

void ChessServer::sendColors()
{
    QJsonObject obj;
    obj["type"] = "color";
    obj["data"] = Engine::toJson(myColor);
    emit message(obj);

    // TODO: multi-color support
    obj["data"] = Engine::toJson(Engine::nextColor(myColor));
    peer->send(obj);
}

void ChessServer::sendBoth(const QJsonObject &obj)
{
    peer->send(obj);
    emit message(obj);
}

bool ChessServer::place(chess_t color, int row, int col)
{
    if (row >= 0 && row < board.count() && col >= 0 && col < board[0].count())
    {
        if (board[row][col] == CH_SPACE)
        {
            board[row][col] = color;
            return true;
        }
    }
    return false;
}

ChessServer::~ChessServer()
{
    close();
}

void ChessServer::startCheckAlive(int interval)
{
    stopCheckAlive();
    timerId = startTimer(interval);
}

void ChessServer::stopCheckAlive()
{
    if (timerId > 0)
    {
        killTimer(timerId);
        timerId = 0;
    }
}

// HTTP

void ChessServer::onHttpRequest(JsonSession *sess, QString header)
{
    if (sess == peer)
    {
        // return;
    }
    QStringList headers = header.split("\r\n", QString::SkipEmptyParts);
    if (headers.count() < 1)
    {
        sess->close();
        sess->deleteLater();
        sess = nullptr;
    }
    QStringList methodAndPathAndVersion = headers.first().split(" ");
    if (methodAndPathAndVersion.count() != 3)
    {
        QByteArray data = QString("<h1>Bad Request</h1>").toUtf8();
        sess->sendHttpResponse(400, "Bad Request");
        sess->sendHttpResponse("Server", "GMKU/0.1");
        sess->sendHttpResponse("Content-Length", QString::number(data.count()));
        sess->sendHttpResponse("Connection", "Close");
        sess->sendHttpResponse();
        sess->sock->write(data);
        removeSession(sess, true);
        return;
    }

    QString method = methodAndPathAndVersion[0],
            path = methodAndPathAndVersion[1],
            version = methodAndPathAndVersion[2];

    qDebug() << method << path << version;

    QByteArray data = QString("<h1>It works!</h1>").toUtf8();
    sess->sendHttpResponse(200, "OK");
    sess->sendHttpResponse("Server", "GMKU/0.1");
    sess->sendHttpResponse("Content-Length", QString::number(data.count()));
    sess->sendHttpResponse("Connection", "Keep-Alive");
    sess->sendHttpResponse();
    sess->sock->write(data);
    // removeSession(sess, true);
}
