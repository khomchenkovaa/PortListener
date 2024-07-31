#include "udphandler.h"

#include <QNetworkDatagram>

/********************************************************/

UdpHandler::UdpHandler(QObject *parent)
    : MessageHandler(tr("UDP handler"), parent)
    , m_UdpSocket(Q_NULLPTR)
{
}

/********************************************************/

UdpHandler::~UdpHandler()
{
    UdpHandler::doDisconnect();
}

/********************************************************/

void UdpHandler::handleMessage(Message *msg)
{
    if (msg->payload.type() == QVariant::ByteArray) {
        processData(msg->payload.toByteArray());
    } else {
        processData(msg->payload.toString());
    }
}

/********************************************************/

QByteArray UdpHandler::processData(const QByteArray &data)
{
    const QString host = settings()->value(Settings::Host, "localhost").toString();
    const quint16 port = settings()->value(Settings::Port, "2424").toUInt();
    m_UdpSocket->writeDatagram(QNetworkDatagram(data, QHostAddress(host), port));
    return QByteArray();
}

/********************************************************/

QByteArray UdpHandler::processData(const QString &data)
{
    const QString host = settings()->value(Settings::Host, "localhost").toString();
    const quint16 port = settings()->value(Settings::Port, "2424").toUInt();
    m_UdpSocket->writeDatagram(QNetworkDatagram(data.toUtf8(), QHostAddress(host), port));
    return QByteArray();
}

/********************************************************/

void UdpHandler::doConnect(bool binary)
{
    Q_UNUSED(binary)

    clearErrors();
    const QString host = settings()->value(Settings::Host, "localhost").toString();
    const quint16 port = settings()->value(Settings::Port, "2424").toUInt();

    m_UdpSocket = new QUdpSocket(this);
    m_UdpSocket->connectToHost(QHostAddress(host), port, QIODevice::WriteOnly);
    setConnected();
}

/********************************************************/

void UdpHandler::doDisconnect()
{
    if (m_UdpSocket) {
        m_UdpSocket->disconnectFromHost();
        m_UdpSocket->deleteLater();
        m_UdpSocket = Q_NULLPTR;
    }
    setDisconnected();
}

/********************************************************/
