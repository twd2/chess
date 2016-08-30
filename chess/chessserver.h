#ifndef CHESSSERVER_H
#define CHESSSERVER_H

#include "jsonsession.h"

#include <functional>

#include <QObject>
#include <QTcpServer>
#include <QHostAddress>
#include <QJsonObject>

class ChessServer
    : QObject
{
    Q_OBJECT
public:
    std::function<bool (QHostAddress, quint16)> grantFunc;
    ChessServer(QHostAddress, quint16, QObject *parent = nullptr);
    ~ChessServer();
    void start();
    void close();
    void command(const QJsonObject &);
signals:
    void message(QJsonObject);
public slots:
    void onNewConnection();
    void onClientMessage(QJsonObject);
    void onMessage(char who, const QJsonObject &);
private:
    char myColor = 'B';
    char turn = 'B';
    QVector<QVector<char> > board;
    QHostAddress address;
    quint16 port;
    QTcpServer *listener = nullptr;
    JsonSession *js = nullptr;

    bool place(char color, int row, int col);
    void sendWin(char);
    void sendColors();
    void sendBoardBoth(int lastRow = -1, int lastCol = -1);
    void sendBoth(const QJsonObject &);
};

#endif // CHESSSERVER_H
