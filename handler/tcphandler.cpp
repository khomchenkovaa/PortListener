#include "tcphandler.h"

#include <QNetworkDatagram>

/********************************************************/

TcpHandler::TcpHandler(QObject *parent)
    : MessageHandler(parent)
    , m_TcpSocket(Q_NULLPTR)
{
    m_Name = tr("TCP handler");
}

/********************************************************/

TcpHandler::~TcpHandler()
{
    doDisconnect();
}

/********************************************************/

void TcpHandler::handleMessage(Message *msg)
{
    if (msg->payload.type() == QVariant::ByteArray) {
        processData(msg->payload.toByteArray());
    } else {
        processData(msg->payload.toString());
    }
}

/********************************************************/

QByteArray TcpHandler::processData(const QByteArray &data)
{
    int sent = m_TcpSocket->write(data, data.length());
    qDebug() << sent << "from" << data;
    return QByteArray();
}

/********************************************************/

QByteArray TcpHandler::processData(const QString &data)
{
    int sent = m_TcpSocket->write(data.toUtf8());
    qDebug() << sent << "from" << data;
    return QByteArray();
}

/********************************************************/

void TcpHandler::doConnect(bool binary)
{
    Q_UNUSED(binary)

    m_Error.clear();
    QString host = m_Settings.value(Settings::Host, "localhost").toString();
    int port = m_Settings.value(Settings::Port, "2424").toInt();

    m_TcpSocket = new QTcpSocket(this);
    QObject::connect(m_TcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, [this](){
        m_Error = m_TcpSocket->errorString();
        qDebug() << m_TcpSocket->errorString();
    });
    m_TcpSocket->connectToHost(host, port);
    m_TcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption,1);

    m_Connected = true;
}

/********************************************************/

void TcpHandler::doDisconnect()
{
    if (m_TcpSocket) {
        m_TcpSocket->disconnectFromHost();
        m_TcpSocket->deleteLater();
        m_TcpSocket = Q_NULLPTR;
    }
}

/********************************************************/
