#include "chessserver.h"

#include <QTcpSocket>

ChessServer::ChessServer(QHostAddress address, quint16 port, QObject *parent)
    : QObject(parent), address(address), port(port)
{
    grantFunc = [] (QHostAddress, quint16) { return true; };
}

bool ChessServer::start()
{
    close();
    listener = new QTcpServer(this);
    bool succeeded = listener->listen(address, port);
    connect(listener, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    return succeeded;
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

void ChessServer::onClientMessage(JsonSession *, QJsonObject obj)
{
    // TODO: multi-color support
    onMessage(Engine::nextColor(myColor), obj);
}

void ChessServer::command(const QJsonObject &obj)
{
    onMessage(myColor, obj);
}

void ChessServer::onMessage(chess_t who, const QJsonObject &obj)
{
    QString type = obj["type"].toString();
    qDebug() << "server on message" << who << obj;
    if (type == "hello")
    {
        // nothing to do
    }
    else if (type == "place")
    {
        if (!isPlaying || turn != who)
        {
            // reject
            return;
        }
        QJsonObject data = obj["data"].toObject();
        int row = data["row"].toInt(),
            col = data["col"].toInt();
        if (!place(who, row, col))
        {
            // reject
            return;
        }

        // next turn
        turn = Engine::nextColor(turn);
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
        if (who != myColor)
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
        if (who != myColor)
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

// HTTP

void ChessServer::onHttpRequest(JsonSession *sess, QString header)
{
    sess->sendHttpResponse(200, "ok");
    sess->sendHttpResponse("Server", "GMKU/0.1");
    sess->sendHttpResponse("Connection", "close");
    sess->sendHttpResponse();
    sess->sock->write(header.toUtf8());
    sess->deleteLater();
    sess = nullptr;
    // sess->close();
}
