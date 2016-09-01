#include "serverdiscovery.h"

ServerDiscovery::ServerDiscovery(QObject *parent)
    : QObject(parent)
{

}

void ServerDiscovery::findServer()
{
    QByteArray buffer = QString("Where is the server?").toUtf8();
    qDebug() << sock->writeDatagram(buffer, multicastGroup, GAME_PORT);
}

void ServerDiscovery::start()
{
    if (isRunning)
    {
        return;
    }
    qDebug() << "run..." << this << this->parent();
    isRunning = true;
    timer = new QTimer(this);

    quint16 port = GAME_PORT;
    if (!isServer)
    {
        // dynamic
        port = 0;
    }

    multicastGroup = QHostAddress("ff1e::5b26");
    if (!tryBindAndJoinMulticastGroup(port))
    {
        // fallback
        multicastGroup = QHostAddress("224.0.91.38");
        tryBindAndJoinMulticastGroup(port);
    }

    if (isServer)
    {
        connect(sock, SIGNAL(readyRead()), this, SLOT(serverOnDatagram()));
    }
    else
    {
        connect(sock, SIGNAL(readyRead()), this, SLOT(clientOnDatagram()));
    }

    if (!isServer)
    {
        connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
        timer->start(1000);
    }
}

void ServerDiscovery::onTimer()
{
    qDebug() << "Tick..";
    if (isServer)
    {
        // sendMulticast("Here I am.");
    }
    else
    {
        findServer();
    }
}

void ServerDiscovery::stop()
{
    if (sock)
    {
        sock->close();
        sock->deleteLater();
        sock = nullptr;
    }

    if (timer)
    {
        timer->stop();
        timer->deleteLater();
        timer = nullptr;
    }
    isRunning = false;
}

void ServerDiscovery::setServerListenAddress(QHostAddress address)
{
    _serverAddress = address;
//    if (address.protocol() == QAbstractSocket::IPv4Protocol)
//    {
//        multicastGroup = QHostAddress("224.0.91.38");
//    }
//    else
//    {
//        // any or ipv6
//        multicastGroup = QHostAddress("ff1e::5b26");
//    }
}

void ServerDiscovery::setServerPort(quint16 port)
{
    _serverPort = port;
}

void ServerDiscovery::sendMulticast(const QString &msg)
{
    QByteArray buffer = QString(msg).toUtf8();
    qDebug() << sock->writeDatagram(buffer, multicastGroup, GAME_PORT);
}

bool ServerDiscovery::tryBindAndJoinMulticastGroup(quint16 port)
{
    if (sock)
    {
        sock->close();
        sock->deleteLater();
        sock = nullptr;
    }

    sock = new QUdpSocket(this);
    if (multicastGroup.protocol() == QAbstractSocket::IPv4Protocol)
    {
        if (!sock->bind(QHostAddress::AnyIPv4, port))
        {
            return false;
        }
    }
    else
    {
        if (!sock->bind(port))
        {
            return false;
        }
    }

    if (!sock->joinMulticastGroup(multicastGroup))
    {
        return false;
    }
    qDebug() << "joined" << multicastGroup;
    return true;
}

void ServerDiscovery::clientOnDatagram()
{
    QByteArray buffer;
    QHostAddress address;
    quint16 port;
    while (sock->pendingDatagramSize() > 0)
    {
        // use the last datagram
        buffer.resize(sock->pendingDatagramSize());
        sock->readDatagram(buffer.data(), buffer.size(), &address, &port);
    }
    qDebug() << "clientOnDatagram" << buffer << address << port;
    QString data = QString::fromUtf8(buffer);
    if (data.mid(0, QString("Here I am").length()) != "Here I am")
    {
        return;
    }
    QStringList list = data.split(",");
    if (list.count() != 3)
    {
        return;
    }
    qDebug() << list[1].trimmed() << list[2].trimmed().toInt();
    emit serverFound(list[1].trimmed(), list[2].trimmed().remove(".").toInt());
}

void ServerDiscovery::serverOnDatagram()
{
    QByteArray buffer;
    QHostAddress address;
    quint16 port;
    while (sock->pendingDatagramSize() > 0)
    {
        // use the last datagram
        buffer.resize(sock->pendingDatagramSize());
        sock->readDatagram(buffer.data(), buffer.size(), &address, &port);
    }
    qDebug() << "serverOnDatagram" << buffer << address << port;
    if (QString::fromUtf8(buffer) != "Where is the server?")
    {
        return;
    }

    buffer = QString("Here I am, %1, %2.").arg(_serverAddress.toString()).arg(_serverPort).toUtf8();
    qDebug() << sock->writeDatagram(buffer, address, port);
}
