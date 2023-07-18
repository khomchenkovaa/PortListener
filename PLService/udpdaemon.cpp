#include "udpdaemon.h"

#include "filehandler.h"
#include "dbhandler.h"

#include "FileOptions.h"
#include "DatabaseOptions.h"

#include <QNetworkDatagram>

/*******************************************************************/

UdpDaemon::UdpDaemon(quint16 port, QObject *parent):
    QObject(parent),
    disabled(false),
    m_UdpSocket(Q_NULLPTR)
{
    initHandler();
    startServer(port);
}

/*******************************************************************/

UdpDaemon::~UdpDaemon()
{
    stopServer();
    qDeleteAll(m_Handlers);
    m_Handlers.clear();
}

/*******************************************************************/

bool UdpDaemon::isListening() const
{
    return m_UdpSocket;
}

/*******************************************************************/

void UdpDaemon::startServer(quint16 port)
{
    for(auto handler : m_Handlers) {
        handler->doConnect();
    }

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
    for(auto handler : m_Handlers) {
        handler->doDisconnect();
    }

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
    MessageHandler* fileHandler = new FileHandler(this);
    SettingsMap map;
    auto fileOptions = Settings::FileOptions::get(QSettings::SystemScope);
    map.insert(FileHandler::FileName, fileOptions.fileName);
    map.insert(FileHandler::FileAppend, fileOptions.fileAppend);
    fileHandler->setSettings(map);
    m_Handlers.append(fileHandler);

    MessageHandler* dbHandler = new DbHandler(this);
    auto dbOptions = Settings::DatabaseOptions::get(QSettings::SystemScope);

    map.clear();
    map.insert(DbHandler::DbHostname, dbOptions.host);
    map.insert(DbHandler::DbPort,     dbOptions.port);
    map.insert(DbHandler::DbDriver,   dbOptions.driver);
    map.insert(DbHandler::DbUsername, dbOptions.username);
    map.insert(DbHandler::DbPassword, dbOptions.password);
    map.insert(DbHandler::DbDatabase, dbOptions.database);
    dbHandler->setSettings(map);
    m_Handlers.append(dbHandler);

//    fileOptions.save(QSettings::SystemScope);
//    dbOptions.save(QSettings::SystemScope);
}

/*******************************************************************/

QByteArray UdpDaemon::processData(const QByteArray &data)
{
    QByteArray reply;
    // Handlers
    for(auto handler : m_Handlers) {
        reply.append(handler->processData(QString(data)));
        if (handler->hasError()) {
            reply.append(handler->lastError().toUtf8());
        }
    }

    // make reply
    if (reply.isEmpty()) {
        reply = "ok\n";
    }

    return reply;
}

/*******************************************************************/
