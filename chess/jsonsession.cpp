#include "jsonsession.h"

#include <QDebug>
#include <QJsonObject>
#include <QtEndian>

constexpr quint32 JsonSession::maxLength;
constexpr quint32 JsonSession::bufferSize;

JsonSession::JsonSession(QTcpSocket *sock, QObject *parent)
    : QObject(parent), sock(sock), lastActive(QDateTime::currentDateTimeUtc())
{
    sock->setParent(this);
    updateActive();
    connect(sock, SIGNAL(readyRead()), this, SLOT(read()));
    connect(sock, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    // echo test: connect(this, SIGNAL(onMessage(QJsonObject)), this, SLOT(send(QJsonObject)));
}

inline quint32 min(const quint32 &a, const quint32 &b)
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
                header->length = qFromBigEndian(header->length);
                qDebug() << header->magic[0] << header->magic[1] << header->magic[2] << header->magic[3] << header->length;
                if (!(header->magic[0] == MAGIC_HEADER[0] && header->magic[1] == MAGIC_HEADER[1] && header->magic[2] == MAGIC_HEADER[2] && header->magic[3] == MAGIC_HEADER[3])
                    || header->length == 0 || header->length > maxLength)
                {
                    qDebug() << "protocol mismatch";
                    close();
                    return;
                }
                status = status_t::readingData;
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
                emit onMessage(QJsonDocument::fromJson(QByteArray::fromRawData(reinterpret_cast<const char *>(buffer), header->length)).object());
                delete buffer;
                delete header;
                header = nullptr;
                buffer = nullptr;
                current_pos = 0;
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
    strcpy(header.magic, MAGIC_HEADER);
    header.length = qToBigEndian(data.count());
    sock->write(reinterpret_cast<const char *>(&header), sizeof(package_header));
    sock->write(data);
}

void JsonSession::close(int wait)
{
    qDebug() << "closing";
    running = false;
    if (sock->isWritable())
    {
        sock->waitForBytesWritten(wait);
    }

    sock->close();
}

void JsonSession::disconnected()
{
    qDebug() << "disconnected";
    running = false;

    sock->close();

    QJsonObject obj;
    obj["type"] = "close";
    emit onMessage(obj);
}

void JsonSession::error(QAbstractSocket::SocketError err)
{
    qDebug() << "error" << err;
    QJsonObject obj;
    obj["type"] = "error";
    obj["data"] = err;
    emit onMessage(obj);
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
