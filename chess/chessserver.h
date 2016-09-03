#ifndef CHESSSERVER_H
#define CHESSSERVER_H

#include "engine.h"
#include "session.h"
#include "jsonsession.h"

#include <functional>

#include <QObject>
#include <QTcpServer>
#include <QHostAddress>
#include <QJsonObject>
#include <QMap>

class ChessServer
    : public QObject
{
    Q_OBJECT
public:
    static constexpr int timeoutInterval = 45000;
    std::function<bool (QHostAddress, quint16)> grantFunc;
    QTcpServer *listener = nullptr;
    ChessServer(QHostAddress, quint16, QObject *parent = nullptr);
    ~ChessServer();
    bool start();
    void close();
    void command(const QJsonObject &);
protected:
    void timerEvent(QTimerEvent *) override;
signals:
    void message(QJsonObject);
public slots:
    void onNewConnection();
    void removeSession(JsonSession *, bool wait = false);
    void onClientMessage(JsonSession *, QJsonObject);
    void onHttpRequest(JsonSession *, QString);
    void onMessage(char who, const QJsonObject &);
    void startCheckAlive(int interval = timeoutInterval);
    void stopCheckAlive();
private:
    chess_t myColor = CH_BLACK;
    chess_t turn = CH_BLACK;
    bool isPlaying = false;
    board_t board;
    int lastRow = -1, lastCol = -1;
    QHostAddress address;
    quint16 port;
    JsonSession *peer = nullptr;
    QMap<JsonSession *, Session> sessions;
    int timerId = 0;

    void startGame();
    bool place(chess_t color, int row, int col);
    void broadcastWin(chess_t);
    void sendColors();
    void sendBoard(JsonSession *sess);
    void broadcastBoard(bool inc = false);
    void broadcast(const QJsonObject &);
};

#endif // CHESSSERVER_H
