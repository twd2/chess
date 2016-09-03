#include "jsonsession.h"

#include <QDebug>
#include <QJsonObject>
#include <QtEndian>

constexpr int JsonSession::heartbeatInterval;
constexpr size_t JsonSession::maxLength;
constexpr size_t JsonSession::bufferSize;

QDateTime JsonSession::lastActive() const
{
    return _lastActive;
}

JsonSession::JsonSession(QTcpSocket *sock, QObject *parent)
    : QObject(parent), sock(sock), _lastActive(QDateTime::currentDateTimeUtc())
{
    sock->setParent(this);
    updateActive();
    connect(sock, SIGNAL(connected()), this, SLOT(connected()));
    connect(sock, SIGNAL(readyRead()), this, SLOT(read()));
    connect(sock, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    // echo test: connect(this, SIGNAL(onMessage(QJsonObject)), this, SLOT(send(QJsonObject)));
}

template <typename T, typename U>
constexpr inline auto min(const T &a, const U &b) -> decltype(a + b)
{
    return a < b ? a : b;
}

void JsonSession::read()
{
    if (!running)
    {
        return;
    }
    bool read = true;
    while (read)
    {
        switch (status)
        {
        case status_t::readingHeader:
        {
            if (!buffer)
            {
                buffer = new quint8[sizeof(package_header)];
            }
            QByteArray data = sock->read(min(sizeof(package_header) - current_pos, bufferSize));
            if (data.count() == 0)
            {
                read = false;
                continue;
            }
            memcpy(buffer + current_pos, data.data(), data.count());
            current_pos += data.count();
            if (current_pos == sizeof(package_header))
            {
                if (header)
                {
                    delete header;
                }
                header = reinterpret_cast<package_header *>(buffer);
                buffer = nullptr;
                current_pos = 0;
                if (memcmp(header->magic, MAGIC_HEADER, sizeof(header->magic)) == 0)
                {
                    header->length = qFromBigEndian(header->length);
                    qDebug() << "length=" << header->length;
                    if (header->length > maxLength)
                    {
                        qDebug() << "data is too long";
                        close();
                        return;
                    }

                    if (header->length == 0)
                    {
                        delete header;
                        header = nullptr;

                        QJsonObject obj;
                        obj["type"] = "null";
                        emit onMessage(this, obj);
                    }
                    else
                    {
                        status = status_t::readingData;
                    }
                }
                else if (memcmp(header->magic, HTTP_HEADER_HEAD, sizeof(header->magic)) == 0
                         || memcmp(header->magic, HTTP_HEADER_GET, sizeof(header->magic)) == 0
                         || memcmp(header->magic, HTTP_HEADER_POST, sizeof(header->magic)) == 0)
                {
                    qDebug() << "HTTP request";
                    char *request = reinterpret_cast<char *>(header);
                    httpHeader.clear();
                    for (int i = 0; i < sizeof(package_header); ++i)
                    {
                        httpHeader += request[i];
                    }
                    delete header;
                    header = nullptr;
                    status = status_t::readingHttpHeader;
                }
                else
                {
                    qDebug() << "protocol mismatch";
                    close();
                    return;
                }
            }
            break;
        }
        case status_t::readingData:
        {
            if (!buffer)
            {
                buffer = new quint8[header->length];
            }
            QByteArray data = sock->read(min(header->length - current_pos, bufferSize));
            if (data.count() == 0)
            {
                read = false;
                continue;
            }
            memcpy(buffer + current_pos, data.data(), data.count());
            current_pos += data.count();
            if (current_pos == header->length)
            {
                emit onMessage(this,
                               QJsonDocument::fromJson(QByteArray::fromRawData(reinterpret_cast<const char *>(buffer), header->length)).object());
                delete header;
                header = nullptr;
                delete [] buffer;
                buffer = nullptr;
                current_pos = 0;
                status = status_t::readingHeader;
            }
            break;
        }
        case status_t::readingHttpHeader:
        {
            QByteArray data = sock->peek(bufferSize);
            if (data.count() == 0)
            {
                read = false;
                continue;
            }
            const QByteArray endHeader("\r\n\r\n");
            int index = data.indexOf(endHeader);
            if (index == -1)
            {
                httpHeader += data;
                sock->read(data.count());
            }
            else
            {
                // endHeader found
                httpHeader += data.mid(0, index + endHeader.count());
                sock->read(index + endHeader.count());
                qDebug() << "http request: " << httpHeader;
                emit onHttpRequest(this, httpHeader);
                status = status_t::readingHeader;
            }
            break;
        }
        default:
            qDebug() << "???";
            break;
        }
        updateActive();
    }
}

void JsonSession::send(const QJsonObject &obj)
{
    QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    package_header header;
    memcpy(header.magic, MAGIC_HEADER, sizeof(header.magic));
    header.length = qToBigEndian(data.count());

    // assume the buffer would never be full
    sock->write(reinterpret_cast<const char *>(&header), sizeof(package_header));
    sock->write(data);
}

void JsonSession::send()
{
    package_header header;
    memcpy(header.magic, MAGIC_HEADER, sizeof(header.magic));
    header.length = qToBigEndian(0);

    // assume the buffer would never be full
    sock->write(reinterpret_cast<const char *>(&header), sizeof(package_header));
}

void JsonSession::close(int wait)
{
    if (!running)
    {
        return;
    }

    qDebug() << "closing";
    running = false;
    stopHeartbeat();

    if (wait != 0 && sock->isWritable() && sock->state() == QAbstractSocket::ConnectedState)
    {
        // WOULD BLOCK!
        sock->waitForBytesWritten(wait);
    }

    qDebug() << sock->state();
    sock->close();
}

void JsonSession::connected()
{
    QJsonObject obj;
    obj["type"] = "connect";
    emit onMessage(this, obj);
}

void JsonSession::disconnected()
{
    qDebug() << "disconnected";
    running = false;

    sock->close();

    QJsonObject obj;
    obj["type"] = "close";
    emit onMessage(this, obj);
}

void JsonSession::error(QAbstractSocket::SocketError err)
{
    qDebug() << "error" << err;
    QJsonObject obj;
    obj["type"] = "error";
    QJsonObject data;
    data["code"] = err;
    data["message"] = sock->errorString();
    obj["data"] = data;
    emit onMessage(this, obj);
}

void JsonSession::updateActive()
{
    _lastActive = QDateTime::currentDateTimeUtc();
}

JsonSession::~JsonSession()
{
    stopHeartbeat();

    if (buffer)
    {
        delete buffer;
        buffer = nullptr;
    }

    if (header)
    {
        delete header;
        header = nullptr;
    }

    if (sock)
    {
        close();
        sock->deleteLater();
        sock = nullptr;
    }
}

void JsonSession::timerEvent(QTimerEvent *)
{
    if (running && sock && sock->state() == QAbstractSocket::ConnectedState)
    {
        send();
    }
}

void JsonSession::startHeartbeat(int interval)
{
    stopHeartbeat();
    timerId = startTimer(interval);
}

void JsonSession::stopHeartbeat()
{
    if (timerId > 0)
    {
        killTimer(timerId);
        timerId = 0;
    }
}

// HTTP

void JsonSession::sendHttpResponse(int code, const QString &desc)
{
    QByteArray buffer = QString("HTTP/1.1 %1 %2\r\n").arg(code).arg(desc).toUtf8();
    sock->write(buffer);
}

void JsonSession::sendHttpResponse(const QString &header, const QString &value)
{
    QByteArray buffer = QString("%1: %2\r\n").arg(header).arg(value).toUtf8();
    sock->write(buffer);
}

void JsonSession::sendHttpResponse()
{
    QByteArray buffer = QString("\r\n").toUtf8();
    sock->write(buffer);
}

void JsonSession::sendHttpResponse(int code, const QString &desc, const QByteArray &data)
{
    sendHttpResponse(code, desc);
    sendHttpResponse("Server", "GMKU/0.1");
    sendHttpResponse("Content-Length", QString::number(data.count()));
    sendHttpResponse("Connection", "Keep-Alive");
    sendHttpResponse();
    sock->write(data);
}
