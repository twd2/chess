#include "widget.h"
#include "ui_widget.h"
#include "engine.h"
#include "setendpoint.h"

#include <QMessageBox>
#include <QJsonObject>
#include <QHostInfo>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->board, SIGNAL(clicked(int,int)), this, SLOT(boardClicked(int,int)));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnTestBoard_clicked()
{
    QVector<QVector<char> > array;
    for (int i = 0; i < 15; ++i)
    {
        QVector<char> row;
        for (int j = 0; j < 15; ++j)
        {
            row.append(' ');
        }
        array.append(row);
    }
    ui->board->setBoard(array);
}

void Widget::on_btnTestLock_clicked()
{
    ui->board->setLock(!ui->board->getLock(), "Hello");
}

void Widget::boardClicked(int row, int col)
{
    QJsonObject obj;
    obj["type"] = "place";
    QJsonObject data;
    data["row"] = row;
    data["col"] = col;
    obj["data"] = data;
    ui->board->setLock(true, tr("Sending data..."));
    sendToServer(obj);
}

void Widget::onMessage(QJsonObject obj)
{
    qDebug() << "widget on message" << obj;
    QString type = obj["type"].toString();
    if (type == "hello")
    {
        // connected
        ui->board->setLock(true, tr("Waiting for start..."));
    }
    else if (type == "color")
    {
        ui->board->color = myColor = obj["data"].toString()[0].toLatin1();
    }
    else if (type == "update")
    {
        QJsonObject data = obj["data"].toObject();
        ui->board->setBoard(Engine::fromJson(data["board"].toArray()));
        ui->board->setLast(data["row"].toInt(), data["col"].toInt());
        // play music: placed
        if (data["turn"].toString()[0].toLatin1() == myColor)
        {
            ui->board->setLock(false);
        }
        else
        {
            ui->board->setLock(true, tr("Waiting for peer..."));
        }
    }
    else if (type == "win")
    {
        char win = obj["data"].toString()[0].toLatin1();
        if (win == myColor)
        {
            ui->board->setLock(true, tr("You WIN :)"));
        }
        else
        {
            ui->board->setLock(true, tr("You lost :("));
        }
    }
    else if (type == "close")
    {
        QMessageBox::warning(this, tr("Warning"), tr("Disconnected."));
        reset();
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
    QJsonArray a;
    ui->board->setBoard(Engine::fromJson(a));
    ui->board->setLock(true, tr("Please connect."));
    ui->gClient->setEnabled(true);
    ui->gServer->setEnabled(true);
    ui->btnConnect->setEnabled(true);
    ui->btnListen->setEnabled(true);
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
    QHostInfo info;
    if (se.address == QHostAddress::Any)
    {
        for (auto &addr : info.addresses())
        {
            if (!addr.isLoopback())
            {
                ui->labAddress->setText(tr("Address: %1").arg(addr.toString()));
                break;
            }
        }
    }
    else if (se.address == QHostAddress::AnyIPv6)
    {
        for (auto &addr : info.addresses())
        {
            if (!addr.isLoopback() && addr.protocol() == QAbstractSocket::IPv6Protocol)
            {
                ui->labAddress->setText(tr("Address: %1").arg(addr.toString()));
                break;
            }
        }
    }
    else if (se.address == QHostAddress::AnyIPv4)
    {
        for (auto &addr : info.addresses())
        {
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
    connect(server, SIGNAL(message(QJsonObject)), this, SLOT(onMessage(QJsonObject)));
    server->start();
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
    connect(client, SIGNAL(onMessage(QJsonObject)), this, SLOT(onMessage(QJsonObject)));
    client->sock->connectToHost(se.address, se.port);
    ui->btnConnect->setEnabled(false);
}
