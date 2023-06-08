#include "udphandler.h"

#include "udphandlerwidget.h"

#include <QNetworkDatagram>

/********************************************************/

UdpHandler::UdpHandler(QObject *parent)
    : MessageHandler(parent)
    , m_UdpSocket(Q_NULLPTR)
{
    m_Name = tr("UDP handler");
}

/********************************************************/

UdpHandler::~UdpHandler()
{
    doDisconnect();
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
    QString host = m_Settings.value(Settings::Host, "localhost").toString();
    int port = m_Settings.value(Settings::Port, "2424").toInt();
    m_UdpSocket->writeDatagram(QNetworkDatagram(data, QHostAddress(host), port));
    return QByteArray();
}

/********************************************************/

QByteArray UdpHandler::processData(const QString &data)
{
    QString host = m_Settings.value(Settings::Host, "localhost").toString();
    int port = m_Settings.value(Settings::Port, "2424").toInt();
    m_UdpSocket->writeDatagram(QNetworkDatagram(data.toUtf8(), QHostAddress(host), port));
    return QByteArray();
}

/********************************************************/

void UdpHandler::doConnect(bool binary)
{
    Q_UNUSED(binary)

    m_Error.clear();
    QString host = m_Settings.value(Settings::Host, "localhost").toString();
    int port = m_Settings.value(Settings::Port, "2424").toInt();

    m_UdpSocket = new QUdpSocket(this);
    m_UdpSocket->connectToHost(QHostAddress(host), port, QIODevice::WriteOnly);
    m_Connected = true;
}

/********************************************************/

void UdpHandler::doDisconnect()
{
    if (m_UdpSocket) {
        m_UdpSocket->disconnectFromHost();
        m_UdpSocket->deleteLater();
        m_UdpSocket = Q_NULLPTR;
    }
}

/********************************************************/

MessageHandlerWgt *UdpHandler::settingsWidget(QWidget *parent) const
{
    return new UdpHandlerWidget(parent);
}

/********************************************************/
