#include "udpdaemon.h"

#include "filehandler.h"

#include <QNetworkDatagram>

/*******************************************************************/

UdpDaemon::UdpDaemon(quint16 port, QObject *parent):
    QObject(parent),
    disabled(false),
    m_UdpSocket(Q_NULLPTR),
    m_Handler(Q_NULLPTR)
{
    initHandler();
    startServer(port);
}

/*******************************************************************/

UdpDaemon::~UdpDaemon()
{
    stopServer();
}

/*******************************************************************/

bool UdpDaemon::isListening() const
{
    return m_UdpSocket;
}

/*******************************************************************/

void UdpDaemon::startServer(quint16 port)
{
    m_Handler->doConnect();
    if (!isListening()) {
        m_UdpSocket = new QUdpSocket(this);
        if (m_UdpSocket->bind(QHostAddress::Any, port)) {
            connect(m_UdpSocket, &QUdpSocket::readyRead,
                   this, &UdpDaemon::readPendingDatagrams);
            QtServiceBase::instance()->logMessage("Sevice started");
        } else {
            QtServiceBase::instance()->logMessage(tr("Sevice startup error: %1").arg(m_UdpSocket->errorString()));
        }
    } else {
        QtServiceBase::instance()->logMessage("Sevice already started");
    }
}

/*******************************************************************/

void UdpDaemon::stopServer()
{
    m_Handler->doDisconnect();
    if (isListening()) {
        m_UdpSocket->close();
        m_UdpSocket->deleteLater();
        m_UdpSocket = Q_NULLPTR;
        QtServiceBase::instance()->logMessage("Sevice stopped");
    }
}

/*******************************************************************/

void UdpDaemon::pauseServer()
{
    disabled = true;
}

/*******************************************************************/

void UdpDaemon::resumeServer()
{
    disabled = false;
}

/*******************************************************************/

void UdpDaemon::readPendingDatagrams()
{
    while (m_UdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_UdpSocket->receiveDatagram();
        QByteArray replyData = processData(datagram.data());
        if (!replyData.isEmpty()) {
            m_UdpSocket->writeDatagram(datagram.makeReply(replyData));
        }
    }
}

/*******************************************************************/

void UdpDaemon::initHandler()
{
    m_Handler = new FileHandler(this);
    SettingsMap map;
    map.insert(FileHandler::FileName, "/home/khoman/udp_log.txt");
    map.insert(FileHandler::FileAppend, true);
    m_Handler->setSettings(map);
}

/*******************************************************************/

QByteArray UdpDaemon::processData(const QByteArray &data)
{
    QByteArray reply;
    // Handler
    if (m_Handler) {
        reply = m_Handler->processData(QString(data));
        if (m_Handler->hasError()) {
            reply = m_Handler->lastError().toUtf8();
        }
    } else {
        reply = "No handler\n";
    }

    // make reply
    if (reply.isEmpty()) {
        reply = "ok\n";
    }

    return reply;
}

/*******************************************************************/
