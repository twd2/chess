#ifndef ECHOSESSION_H
#define ECHOSESSION_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QString>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

#define MAGIC_HEADER "5CHS"

struct package_header
{
    char magic[4];
    quint32 length;
};

class JsonSession : public QObject
{
    Q_OBJECT
    enum status_t
    {
        readingHeader,
        readingData
    };

public:
    static constexpr quint32 maxLength = 10240;
    static constexpr quint32 bufferSize = 4096;
    QTcpSocket *sock;
    bool running = true;
    QDateTime lastActive;
    explicit JsonSession(QTcpSocket *sock, QObject *parent = 0);
    ~JsonSession();
signals:
    void onMessage(QJsonObject);
public slots:
    void read();
    void disconnected();
    void error(QAbstractSocket::SocketError);
    void close(int wait = 0);
    void send(const QJsonObject &);
    void updateActive();
private:
    status_t status = status_t::readingHeader;
    quint64 current_pos = 0;
    quint8 *buffer = nullptr;
    package_header *header = nullptr;
};

#endif // ECHOSESSION_H
