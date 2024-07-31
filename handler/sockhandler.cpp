#include "sockhandler.h"

/********************************************************/

SockHandler::SockHandler(QObject *parent)
    : MessageHandler(tr("Socket handler"), parent)
    , m_LocalSocket(Q_NULLPTR)
{
}

/********************************************************/

SockHandler::~SockHandler()
{
    SockHandler::doDisconnect();
}

/********************************************************/

QByteArray SockHandler::processData(const QByteArray &data)
{
    m_LocalSocket->write(data);
    return QByteArray();
}

/********************************************************/

QByteArray SockHandler::processData(const QString &data)
{
    m_LocalSocket->write(data.toUtf8());
    return QByteArray();
}

/********************************************************/

void SockHandler::doConnect(bool binary)
{
    Q_UNUSED(binary)

    clearErrors();
    const auto socket = settings()->value(Settings::Socket).toString();

    m_LocalSocket = new QLocalSocket(this);
    m_LocalSocket->connectToServer(socket, QIODevice::WriteOnly);
    setConnected();
}

/********************************************************/

void SockHandler::doDisconnect()
{
    if (m_LocalSocket) {
        m_LocalSocket->disconnectFromServer();
        m_LocalSocket->deleteLater();
        m_LocalSocket = Q_NULLPTR;
    }
    setDisconnected();
}

/********************************************************/
