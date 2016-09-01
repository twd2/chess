#ifndef SERVERDISCOVERY_H
#define SERVERDISCOVERY_H

#include "utils.h"

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QUdpSocket>

class ServerDiscovery : public QObject
{
    Q_OBJECT
public:
    bool isServer = true;
    bool isRunning = false;
    ServerDiscovery(QObject *parent = nullptr);

signals:
    void serverFound(QString, quint16);
public slots:
    void findServer();
    void clientOnDatagram();
    void serverOnDatagram();
    void onTimer();
    void start();
    void stop();
    void setServerListenAddress(QHostAddress);
    void setServerPort(quint16);
private:
    QHostAddress _serverAddress;
    quint16 _serverPort;
    QUdpSocket *sock = nullptr;
    QTimer *timer = nullptr;
    QHostAddress multicastGroup;
    void sendMulticast(const QString &);
    bool tryBindAndJoinMulticastGroup(quint16);
};

#endif // SERVERDISCOVERY_H
