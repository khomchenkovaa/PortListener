#include "sockhandler.h"

#include "sockhandlerwidget.h"

/********************************************************/

SockHandler::SockHandler(QObject *parent)
    : MessageHandler(parent)
    , m_LocalSocket(Q_NULLPTR)
{
    m_Name = tr("Socket handler");
}

/********************************************************/

SockHandler::~SockHandler()
{
    doDisconnect();
}

/********************************************************/

void SockHandler::handleMessage(Message *msg)
{
    if (msg->payload.type() == QVariant::ByteArray) {
        processData(msg->payload.toByteArray());
    } else {
        processData(msg->payload.toString());
    }
}

/********************************************************/

QByteArray SockHandler::processData(const QByteArray &data)
{
    QString socket = m_Settings.value(Settings::Socket).toString();
    m_LocalSocket->write(data);
    return QByteArray();
}

/********************************************************/

QByteArray SockHandler::processData(const QString &data)
{
    QString socket = m_Settings.value(Settings::Socket).toString();
    m_LocalSocket->write(data.toUtf8());
    return QByteArray();
}

/********************************************************/

void SockHandler::doConnect(bool binary)
{
    Q_UNUSED(binary)

    m_Error.clear();
    QString socket = m_Settings.value(Settings::Socket).toString();

    m_LocalSocket = new QLocalSocket(this);
    m_LocalSocket->connectToServer(socket, QIODevice::WriteOnly);
    m_Connected = true;
}

/********************************************************/

void SockHandler::doDisconnect()
{
    if (m_LocalSocket) {
        m_LocalSocket->disconnectFromServer();
        m_LocalSocket->deleteLater();
        m_LocalSocket = Q_NULLPTR;
    }
    m_Connected = false;
}

/********************************************************/

MessageHandlerWgt *SockHandler::settingsWidget(QWidget *parent) const
{
    return new SockHandlerWidget(parent);
}

/********************************************************/
