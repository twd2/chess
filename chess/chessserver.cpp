#include "chessserver.h"
#include "engine.h"

#include <QTcpSocket>

ChessServer::ChessServer(QHostAddress address, quint16 port, QObject *parent)
    : QObject(parent), address(address), port(port)
{
    grantFunc = [] (QHostAddress, quint16) { return true; };
}

void ChessServer::start()
{
    close();
    listener = new QTcpServer(this);
    listener->listen(address, port);
    connect(listener, SIGNAL(newConnection()), this,SLOT(onNewConnection()));
}

void ChessServer::onNewConnection()
{
    QTcpSocket *sock = listener->nextPendingConnection();
    if (js || !grantFunc(sock->peerAddress(), sock->peerPort()))
    {
        sock->close();
        delete sock;
    }
    else
    {
        js = new JsonSession(sock, this);
        connect(js, SIGNAL(onMessage(QJsonObject)), this, SLOT(onClientMessage(QJsonObject)));

        // "hello"s
        QJsonObject objHello;
        objHello["type"] = "hello";
        sendBoth(objHello);

        board = Engine::generate();
        // set up turn, myColor
        sendColors();
        sendBoardBoth();
    }
}

void ChessServer::onClientMessage(QJsonObject obj)
{
    onMessage(Engine::otherColor(myColor), obj);
}

void ChessServer::command(const QJsonObject &obj)
{
    onMessage(myColor, obj);
}

void ChessServer::onMessage(char who, const QJsonObject &obj)
{
    qDebug() << "server on message" << obj;
    QString type = obj["type"].toString();
    if (type == "hello")
    {
        // nothing to do
    }
    else if (type == "place")
    {
        if (turn != who)
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
        turn = Engine::otherColor(turn);
        sendBoardBoth(row, col);

        // check win
        char win = Engine::findWin(board);
        if (win != ' ')
        {
            sendWin(win);
        }
    }
    else if (type == "close")
    {
        if (who != myColor)
        {
            // peer disconnected
            js->deleteLater();
            js = nullptr;
            emit message(obj);
        }
        else
        {
            // ???
        }
    }
}

void ChessServer::close()
{
    if (listener)
    {
        listener->close();
        delete listener;
        listener = nullptr;
    }

    if (js)
    {
        js->close();
        delete js;
        js = nullptr;
    }
}

void ChessServer::sendBoardBoth(int lastRow, int lastCol)
{
    QJsonObject obj;
    obj["type"] = "update";
    QJsonObject data;
    data["row"] = lastRow;
    data["col"] = lastCol;
    data["turn"] = QString(turn);
    data["board"] = Engine::toJson(board);
    obj["data"] = data;
    sendBoth(obj);
}

void ChessServer::sendWin(char win)
{
    QJsonObject obj;
    obj["type"] = "win";
    obj["data"] = QString(win);
    sendBoth(obj);
}

void ChessServer::sendColors()
{
    QJsonObject obj;
    obj["type"] = "color";
    obj["data"] = QString(myColor);
    emit message(obj);

    obj["data"] = QString(Engine::otherColor(myColor));
    js->send(obj);
}

void ChessServer::sendBoth(const QJsonObject &obj)
{
    js->send(obj);
    emit message(obj);
}

bool ChessServer::place(char color, int row, int col)
{
    if (row >= 0 && row < board.count() && col >= 0 && col < board[0].count())
    {
        if (board[row][col] == ' ')
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
