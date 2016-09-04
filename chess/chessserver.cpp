#include "chessserver.h"

#include <QFile>
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
    JsonSession *sess = new JsonSession(sock, this);
    connect(sess, SIGNAL(onMessage(JsonSession *, QJsonObject)), this, SLOT(onClientMessage(JsonSession *, QJsonObject)));
    connect(sess, SIGNAL(onHttpRequest(JsonSession*, QString)), this, SLOT(onHttpRequest(JsonSession*, QString)));
    sessions.insert(sess, Session());
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

void ChessServer::onClientMessage(JsonSession *sess, QJsonObject obj)
{
    bool isPeer = sess == peer;
    QString type = obj["type"].toString();
    qDebug() << "server on message" << sess << obj;
    // universal messages
    if (type == "null")
    {
        // heartbeat
        sess->updateActive();
    }
    else if (type == "join")
    {
        // hello
        QJsonObject obj;
        obj["type"] = "hello";
        sess->send(obj);
        sessions[sess].joined = true;

        bool isViewer = false;

        if (peer || !grantFunc(sess->sock->peerAddress(), sess->sock->peerPort()))
        {
            isViewer = true;
        }
        else
        {
            if (peer)
            {
                isViewer = true;
            }
            else
            {
                peer = sess;
                sessions[sess].isPeer = true;
                startGame();
            }
        }
        if (isViewer)
        {
            QJsonObject obj;
            obj["type"] = "color";
            obj["data"] = Engine::toJson(CH_VIEWER);
            sess->send(obj);

            if (isPlaying)
            {
                // viewer
                sendBoard(sess);
            }
        }
    }
    else if (type == "hello")
    {
        // nothing to do
    }
    else if (type == "close" || type == "error")
    {
        // session disconnected/error
        removeSession(sess);

        if (isPeer)
        {
            isPlaying = false;
            emit message(obj);
        }
    }
    else
    {
        // game-related messages

        if (isPeer)
        {
            onMessage(sessions[sess].color, obj);
        }
        else
        {
            // reject
        }
    }
}

void ChessServer::command(const QJsonObject &obj)
{
    // game-related messages
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
    // game-related messages
    QString type = obj["type"].toString();
    if (type == "place")
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

        lastRow = row;
        lastCol = col;

        broadcastBoard(true);

        // check win
        chess_t win = Engine::findWin(board);
        if (win != CH_SPACE)
        {
            isPlaying = false;
            broadcastWin(win);
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
    broadcast(obj);

    // setup
    board = Engine::generate(15, 15);
    myColor = Engine::randomColor();
    sessions[peer].color = Engine::nextColor(myColor);
    // TODO: multi-color support

    // black first
    turn = CH_BLACK;
    isPlaying = true;
    lastRow = lastCol = -1;

    sendColors();
    broadcastBoard();
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

void ChessServer::broadcastBoard(bool inc)
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
    broadcast(obj);
}

void ChessServer::sendBoard(JsonSession *sess)
{
    QJsonObject obj;
    obj["type"] = "update";
    QJsonObject data;
    data["row"] = lastRow;
    data["col"] = lastCol;
    data["turn"] = Engine::toJson(turn);
    data["board"] = Engine::toJson(board);
    obj["data"] = data;
    sess->send(obj);
}

void ChessServer::broadcastWin(chess_t win)
{
    QJsonObject obj;
    obj["type"] = "win";
    obj["data"] = Engine::toJson(win);
    broadcast(obj);
}

void ChessServer::sendColors()
{
    QJsonObject obj;
    obj["type"] = "color";
    obj["data"] = Engine::toJson(myColor);
    emit message(obj);

    // TODO: multi-color support
    obj["data"] = Engine::toJson(sessions[peer].color);
    peer->send(obj);
}

void ChessServer::broadcast(const QJsonObject &obj)
{
    for (JsonSession *sess : sessions.keys())
    {
        if (sessions[sess].joined)
        {
            sess->send(obj);
        }
    }
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
        // ???
    }
    sessions[sess].isHttp = true;
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
        sess->sendHttpResponse(400, "Bad Request", QString("<h1>Bad Request</h1>").toUtf8(), false);
        removeSession(sess, true);
        return;
    }

    QString method = methodAndPathAndVersion[0],
            path = methodAndPathAndVersion[1],
            version = methodAndPathAndVersion[2];

    qDebug() << method << path << version;

    bool keepAlive = true;
    if (version != "HTTP/1.1")
    {
        keepAlive = false;
    }

    if (path == "/")
    {
        path = "/index.html";
    }

    if (path == "/board")
    {
        QJsonObject data;
        data["row"] = lastRow;
        data["col"] = lastCol;
        data["turn"] = Engine::toJson(turn);
        data["board"] = Engine::toJson(board);
        QByteArray buff = QJsonDocument(data).toJson(QJsonDocument::Compact);
        sess->sendHttpResponse(200, "OK", buff, keepAlive);
    }
    else
    {
        QVector<QString> innerFiles = { "/index.html" };
        if (innerFiles.indexOf(path) != -1)
        {
            QFile f(QString(":/webserver%1").arg(path));
            f.open(QFile::ReadOnly);
            QByteArray data = f.readAll();
            f.close();
            sess->sendHttpResponse(200, "OK", data, keepAlive);
        }
        else
        {
            sess->sendHttpResponse(404, "Not Found", QString("<h1>404 not found</h1>").toUtf8(), keepAlive);
        }
    }

    if (!keepAlive)
    {
        removeSession(sess, true);
    }
}
