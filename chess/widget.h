#ifndef WIDGET_H
#define WIDGET_H

#include "chessserver.h"
#include "jsonsession.h"

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
    void on_btnTestBoard_clicked();
    void on_btnTestLock_clicked();
    void boardClicked(int row, int col);
    void onMessage(QJsonObject);

    void on_btnClientStop_clicked();

    void on_btnServerStop_clicked();

    void on_btnListen_clicked();

    void on_btnConnect_clicked();

    void on_btnHint_toggled(bool checked);

private:
    Ui::Widget *ui;
    ChessServer *server = nullptr;
    JsonSession *client = nullptr;
    char myColor = ' ';

    void sendToServer(const QJsonObject &);
    void reset();
};

#endif // WIDGET_H
