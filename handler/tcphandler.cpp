#include "tcphandler.h"

#include <QNetworkDatagram>

/********************************************************/

TcpHandler::TcpHandler(QObject *parent)
    : MessageHandler(tr("TCP handler"), parent)
    , m_TcpSocket(Q_NULLPTR)
{
}

/********************************************************/

TcpHandler::~TcpHandler()
{
    TcpHandler::doDisconnect();
}

/********************************************************/

QByteArray TcpHandler::processData(const QByteArray &data)
{
    m_TcpSocket->write(data, data.length());
    return m_TcpSocket->readAll();
}

/********************************************************/

QByteArray TcpHandler::processData(const QString &data)
{
    m_TcpSocket->write(data.toUtf8());
    return m_TcpSocket->readAll();
}

/********************************************************/

void TcpHandler::doConnect(bool binary)
{
    Q_UNUSED(binary)

    clearErrors();
    const QString host = settings()->value(Settings::Host, "localhost").toString();
    const quint16 port = settings()->value(Settings::Port, "2424").toUInt();

    m_TcpSocket = new QTcpSocket(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QObject::connect(m_TcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, [this](){
            addError(m_TcpSocket->errorString());
            qDebug() << m_TcpSocket->errorString();
        });
#else
    QObject::connect(m_TcpSocket, &QAbstractSocket::errorOccurred, this, [this](){
        addError(m_TcpSocket->errorString());
        qDebug() << m_TcpSocket->errorString();
    });
#endif
    m_TcpSocket->connectToHost(host, port);
    m_TcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    setConnected();
}

/********************************************************/

void TcpHandler::doDisconnect()
{
    if (m_TcpSocket) {
        m_TcpSocket->disconnectFromHost();
        m_TcpSocket->deleteLater();
        m_TcpSocket = Q_NULLPTR;
    }
    setDisconnected();
}

/********************************************************/
