#ifndef WIDGET_H
#define WIDGET_H

#include "ai.h"
#include "chessserver.h"
#include "jsonsession.h"
#include "engine.h"
#include "serverdiscovery.h"

#include <memory>

#include <QWidget>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_btnTestLock_clicked();
    void boardClicked(int row, int col);
    void aiSuggest(int, int, int);
    void onMessage(QJsonObject);
    void onMessage(JsonSession *, QJsonObject);
    void on_btnClientStop_clicked();
    void on_btnServerStop_clicked();
    void on_btnListen_clicked();
    void on_btnConnect_clicked();
    void on_btnHint_toggled(bool checked);
    void on_btnStart_clicked();
    void on_btnDiscovery_clicked();
signals:
    void startDiscovery();
    void stopDiscovery();
    void boardChanged(board_t, chess_t, int);
private:
    Ui::Widget *ui;
    QHostAddress _lastAddress = QHostAddress("127.0.0.1");
    quint16 _lastPort = GAME_PORT;
    ChessServer *server = nullptr;
    JsonSession *client = nullptr;
    AI *ai = nullptr;
    ServerDiscovery *discovery = nullptr;
    QThread *workerThread = nullptr;
    char myColor = CH_SPACE;
    board_t _board;

    void sendToServer(const QJsonObject &);
    void reset();
    void resetBoard();
    void setMessage(bool lock, const QString &msg);
    void connectToServer(QHostAddress, quint16);
};

#endif // WIDGET_H
