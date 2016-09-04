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
    static constexpr int heartbeatInterval = 40000;
    static constexpr size_t maxLength = 10240;
    static constexpr size_t bufferSize = 4096;
    QTcpSocket *sock;
    bool running = true;
    explicit JsonSession(QTcpSocket *sock, QObject *parent = 0);
    ~JsonSession();
    QDateTime lastActive() const;

protected:
    void timerEvent(QTimerEvent *) override;
signals:
    void onMessage(JsonSession *, QJsonObject);
    void onHttpRequest(JsonSession *, QString);
public slots:
    void connected();
    void read();
    void disconnected();
    void error(QAbstractSocket::SocketError);
    void close(int wait = 0);
    void send(const QJsonObject &);
    void send();
    void updateActive();
    void sendHttpResponse(int code, const QString &desc);
    void sendHttpResponse(const QString &header, const QString &value);
    void sendHttpResponse();
    void sendHttpResponse(int code, const QString &desc, const QByteArray &data, bool keepAlive = true);
    void startHeartbeat(int interval = heartbeatInterval);
    void stopHeartbeat();
private:
    QDateTime _lastActive;
    status_t status = status_t::readingHeader;
    quint64 current_pos = 0;
    quint8 *buffer = nullptr;
    package_header *header = nullptr;
    QString httpHeader;
    int timerId = 0;
};

#endif // ECHOSESSION_H
