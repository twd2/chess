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

#define MAGIC_HEADER "GMKU"
#define HTTP_HEADER_HEAD "HEAD"
#define HTTP_HEADER_GET "GET "
#define HTTP_HEADER_POST "POST"

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
        readingData,
        readingHttpHeader
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
    void onMessage(JsonSession *, QJsonObject);
    void onHttpRequest(JsonSession *, QString);
public slots:
    void read();
    void disconnected();
    void error(QAbstractSocket::SocketError);
    void close(int wait = 0);
    void send(const QJsonObject &);
    void updateActive();
    void sendHttpResponse(int code, QString desc);
    void sendHttpResponse(QString header, QString value);
    void sendHttpResponse();
private:
    status_t status = status_t::readingHeader;
    quint64 current_pos = 0;
    quint8 *buffer = nullptr;
    package_header *header = nullptr;
    QString httpHeader;
};

#endif // ECHOSESSION_H
