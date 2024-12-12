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
    if (!isConnected()) {
        addError("Not connected to Host");
        return QByteArray();
    }
    m_TcpSocket->write(data, data.length());
    if (!m_TcpSocket->waitForBytesWritten()) {
        addError("Cannot write binary data");
        return QByteArray();
    }
    return m_TcpSocket->readAll();
}

/********************************************************/

QByteArray TcpHandler::processData(const QString &data)
{
    if (!isConnected()) {
        addError("Not connected to Host");
        return QByteArray();
    }
    m_TcpSocket->write(data.toUtf8());
    if (!m_TcpSocket->waitForBytesWritten()) {
        addError("Cannot write text data");
        return QByteArray();
    }
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

    if (m_TcpSocket->waitForConnected()) {
        setConnected();
    } else {
        addError("Connection error");
    }
}

/********************************************************/

void TcpHandler::doDisconnect()
{
    if (m_TcpSocket) {
        m_TcpSocket->close();
        m_TcpSocket->deleteLater();
        m_TcpSocket = Q_NULLPTR;
    }
    setDisconnected();
}

/********************************************************/
