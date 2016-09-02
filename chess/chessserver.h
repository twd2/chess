#ifndef CHESSSERVER_H
#define CHESSSERVER_H

#include "engine.h"
#include "jsonsession.h"

#include <functional>

#include <QObject>
#include <QTcpServer>
#include <QHostAddress>
#include <QJsonObject>

class ChessServer
    : public QObject
{
    Q_OBJECT
public:
    std::function<bool (QHostAddress, quint16)> grantFunc;
    QTcpServer *listener = nullptr;
    ChessServer(QHostAddress, quint16, QObject *parent = nullptr);
    ~ChessServer();
    bool start();
    void close();
    void command(const QJsonObject &);
signals:
    void message(QJsonObject);
public slots:
    void onNewConnection();
    void onClientMessage(JsonSession *, QJsonObject);
    void onHttpRequest(JsonSession *, QString);
    void onMessage(char who, const QJsonObject &);
private:
    chess_t myColor = CH_BLACK;
    chess_t turn = CH_BLACK;
    bool isPlaying = false;
    board_t board;
    QHostAddress address;
    quint16 port;
    JsonSession *peer = nullptr;

    void startGame();
    bool place(chess_t color, int row, int col);
    void sendWin(chess_t);
    void sendColors();
    void sendBoardBoth(int lastRow = -1, int lastCol = -1, bool inc = false);
    void sendBoth(const QJsonObject &);
};

#endif // CHESSSERVER_H
