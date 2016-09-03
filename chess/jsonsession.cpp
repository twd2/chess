#include "jsonsession.h"

#include <QDebug>
#include <QJsonObject>
#include <QtEndian>

constexpr size_t JsonSession::maxLength;
constexpr size_t JsonSession::bufferSize;

JsonSession::JsonSession(QTcpSocket *sock, QObject *parent)
    : QObject(parent), sock(sock), lastActive(QDateTime::currentDateTimeUtc())
{
    sock->setParent(this);
    updateActive();
    connect(sock, SIGNAL(connected()), this, SLOT(connected()));
    connect(sock, SIGNAL(readyRead()), this, SLOT(read()));
    connect(sock, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    // echo test: connect(this, SIGNAL(onMessage(QJsonObject)), this, SLOT(send(QJsonObject)));
}

template <typename T>
constexpr inline T min(const T &a, const T &b)
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
                    if (header->length == 0 || header->length > maxLength)
                    {
                        qDebug() << "data is null or too long";
                        close();
                        return;
                    }
                    status = status_t::readingData;
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
                delete buffer;
                delete header;
                header = nullptr;
                buffer = nullptr;
                current_pos = 0;
                status = status_t::readingHeader;
            }
            break;
        }
        case status_t::readingHttpHeader:
        {
            while (true)
            {
                if (httpHeader.length() >= maxLength)
                {
                    qDebug() << "http header is too long";
                    close();
                    return;
                }
                char ch;
                if (sock->read(&ch, sizeof(ch)) <= 0)
                {
                    read = false;
                    break;
                }
                httpHeader += ch;
                if (httpHeader.endsWith("\r\n\r\n") || httpHeader.endsWith("\n\n"))
                {
                    qDebug() << "http request: " << httpHeader;
                    emit onHttpRequest(this, httpHeader);
                    status = status_t::readingHeader;
                    break;
                }
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
    strcpy(header.magic, MAGIC_HEADER);
    header.length = qToBigEndian(data.count());

    // assume the buffer would never be full
    sock->write(reinterpret_cast<const char *>(&header), sizeof(package_header));
    sock->write(data);
}

void JsonSession::close(int wait)
{
    if (!running)
    {
        return;
    }
    qDebug() << "closing";
    running = false;
    if (sock->isWritable() && sock->state() == QAbstractSocket::SocketState::ConnectedState)
    {
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
    lastActive = QDateTime::currentDateTimeUtc();
}

JsonSession::~JsonSession()
{
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

// HTTP

void JsonSession::sendHttpResponse(int code, QString desc)
{
    QByteArray buffer = QString("HTTP/1.1 %1 %2\r\n").arg(code).arg(desc).toUtf8();
    sock->write(buffer);
}

void JsonSession::sendHttpResponse(QString header, QString value)
{
    QByteArray buffer = QString("%1: %2\r\n").arg(header).arg(value).toUtf8();
    sock->write(buffer);
}

void JsonSession::sendHttpResponse()
{
    QByteArray buffer = QString("\r\n").toUtf8();
    sock->write(buffer);
}
