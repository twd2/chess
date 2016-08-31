#include "widget.h"
#include "ui_widget.h"
#include "setendpoint.h"

#include <QMessageBox>
#include <QJsonObject>
#include <QNetworkInterface>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    reset();
    connect(ui->board, SIGNAL(clicked(int,int)), this, SLOT(boardClicked(int,int)));
    ui->label_2->setText(tr("By Wandai :) Enjoy~"));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnTestLock_clicked()
{
    setMessage(!ui->board->lock(), "Hello");
}

void Widget::boardClicked(int row, int col)
{
    QJsonObject obj;
    obj["type"] = "place";
    QJsonObject data;
    data["row"] = row;
    data["col"] = col;
    obj["data"] = data;
    setMessage(true, tr("Sending data..."));
    sendToServer(obj);
}

void Widget::onMessage(QJsonObject obj)
{
    QString type = obj["type"].toString();
    qDebug() << "widget on message" << obj;
    if (type == "hello")
    {
        // connected
        setMessage(true, tr("Waiting for start..."));
    }
    else if (type == "color")
    {
        ui->board->myColor = myColor = Engine::fromJson(obj["data"]);
        ui->labColor->setText(tr("My color: %1").arg(Engine::name(myColor)));
    }
    else if (type == "update")
    {
        QJsonObject data = obj["data"].toObject();
        chess_t turn = Engine::fromJson(data["turn"]);
        if (data.contains("board"))
        {
            // full update
            _board = Engine::fromJson(data["board"].toArray());
        }
        else
        {
            // inc update
            _board[data["row"].toInt()][data["col"].toInt()] = Engine::previousColor(turn);
        }
        ui->board->setBoard(_board);
        ui->board->setLast(data["row"].toInt(), data["col"].toInt());
        // play music: placed
        if (turn == myColor)
        {
            setMessage(false, tr("It's your turn."));

            // auto
            if (ui->btnAuto->isChecked())
            {
                QPoint p = Engine::findMostDangerous(_board, myColor);
                qDebug() << p;
                boardClicked(p.y(), p.x());
            }
        }
        else
        {
            setMessage(true, tr("Waiting for peer..."));
        }
    }
    else if (type == "win")
    {
        char win = obj["data"].toString()[0].toLatin1();
        if (win == '-')
        {
            setMessage(true, tr("Draw O.O"));
        }
        else if (win == myColor)
        {
            setMessage(true, tr("You WIN :)"));
        }
        else
        {
            setMessage(true, tr("You lose :("));
        }
    }
    else if (type == "close")
    {
        if (client)
        {
            // client mode
            reset();
        }
        else
        {
            // server mode
            resetBoard();
        }
        QMessageBox::warning(this, tr("Warning"), tr("Disconnected."));
    }
}

void Widget::sendToServer(const QJsonObject &obj)
{
    if (server)
    {
        server->command(obj);
    }

    if (client)
    {
        client->send(obj);
    }
}

void Widget::reset()
{
    if (client)
    {
        client->close();
        client->deleteLater();
        client = nullptr;
    }
    if (server)
    {
        server->close();
        server->deleteLater();
        server = nullptr;
    }
    ui->gClient->setEnabled(true);
    ui->gServer->setEnabled(true);
    ui->btnConnect->setEnabled(true);
    ui->btnListen->setEnabled(true);
    ui->labColor->setText(tr("My color: ?"));
    resetBoard();
}

void Widget::resetBoard()
{
    QJsonArray a;
    ui->board->setBoard(Engine::fromJson(a));
    setMessage(true, tr("Please connect or start a server."));
}

void Widget::setMessage(bool lock, const QString &msg)
{
    if (lock)
    {
        ui->board->setLock(true, msg);
    }
    else
    {
        ui->board->setLock(false);
    }
    ui->labInfo->setText(msg);
}

void Widget::on_btnClientStop_clicked()
{
    reset();
}

void Widget::on_btnServerStop_clicked()
{
    reset();
}

void Widget::on_btnListen_clicked()
{
    SetEndpoint se;
    se.setAddress(QHostAddress::Any);
    se.setPort(23334);
    if (!se.exec())
    {
        return;
    }

    ui->labAddress->setText(tr("Address: ?"));
    if (se.address == QHostAddress::Any)
    {
        ui->labAddress->setText(tr("Address: [::1]"));
        for (auto &addr : QNetworkInterface::allAddresses())
        {
            qDebug() << addr;
            if (!addr.isLoopback())
            {
                ui->labAddress->setText(tr("Address: %1").arg(addr.toString()));
                break;
            }
        }
    }
    else if (se.address == QHostAddress::AnyIPv6)
    {
        ui->labAddress->setText(tr("Address: [::1]"));
        for (auto &addr : QNetworkInterface::allAddresses())
        {
            qDebug() << addr;
            if (!addr.isLoopback() && addr.protocol() == QAbstractSocket::IPv6Protocol)
            {
                ui->labAddress->setText(tr("Address: %1").arg(addr.toString()));
                break;
            }
        }
    }
    else if (se.address == QHostAddress::AnyIPv4)
    {
        ui->labAddress->setText(tr("Address: 127.0.0.1"));
        for (auto &addr : QNetworkInterface::allAddresses())
        {
            qDebug() << addr;
            if (!addr.isLoopback() && addr.protocol() == QAbstractSocket::IPv4Protocol)
            {
                ui->labAddress->setText(tr("Address: %1").arg(addr.toString()));
                break;
            }
        }
    }
    else
    {
        ui->labAddress->setText(tr("Address: %1").arg(se.address.toString()));
    }
    ui->labPort->setText(tr("Port: %1").arg(se.port));
    server = new ChessServer(se.address, se.port, this);
    server->grantFunc = [&] (QHostAddress address, quint16 port) -> bool
    {
        if (QMessageBox::question(this, tr("Connection Request"), tr("Connection from %1:%2, grant?").arg(address.toString()).arg(port),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            return true;
        }
        return false;
    };
    setMessage(true, "Waiting for connection...");
    connect(server, SIGNAL(message(QJsonObject)), this, SLOT(onMessage(QJsonObject)));
    bool succeeded = server->start();
    if (!succeeded)
    {
        QMessageBox::critical(this, tr("Error"), tr("Listen failed: %1").arg(server->listener->errorString()));
        reset();
        return;
    }
    ui->btnListen->setEnabled(false);
}

void Widget::on_btnConnect_clicked()
{
    SetEndpoint se;
    se.setAddress(QHostAddress("127.0.0.1"));
    se.setPort(23334);
    if (!se.exec())
    {
        return;
    }
    client = new JsonSession(new QTcpSocket(), this);
    setMessage(true, "Connecting...");
    connect(client, SIGNAL(onMessage(QJsonObject)), this, SLOT(onMessage(QJsonObject)));
    client->sock->connectToHost(se.address, se.port);
    ui->btnConnect->setEnabled(false);
}

void Widget::on_btnHint_toggled(bool checked)
{
    ui->board->setHint(checked);
}

void Widget::on_btnStart_clicked()
{
    QJsonObject obj;
    obj["type"] = "new";
    sendToServer(obj);
}
