#include "udpdaemon.h"

#include "filehandler.h"
#include "dbhandler.h"

#include "FileOptions.h"
#include "DatabaseOptions.h"

#include <QNetworkDatagram>

/*******************************************************************/

UdpDaemon::UdpDaemon(quint16 port, QObject *parent):
    QObject(parent)
{
    initHandler();
    startServer(port);
}

/*******************************************************************/

UdpDaemon::~UdpDaemon()
{
    stopServer();
    qDeleteAll(d.handlers);
    d.handlers.clear();
}

/*******************************************************************/

bool UdpDaemon::isListening() const
{
    return d.socket;
}

/*******************************************************************/

void UdpDaemon::startServer(quint16 port)
{
    for(auto handler : qAsConst(d.handlers)) {
        handler->doConnect();
    }

    if (!isListening()) {
        d.socket = new QUdpSocket(this);
        if (d.socket->bind(QHostAddress::Any, port)) {
            connect(d.socket, &QUdpSocket::readyRead,
                   this, &UdpDaemon::readPendingDatagrams);
            QtServiceBase::instance()->logMessage("Sevice started");
        } else {
            QtServiceBase::instance()->logMessage(tr("Sevice startup error: %1").arg(d.socket->errorString()));
        }
    } else {
        QtServiceBase::instance()->logMessage("Sevice already started");
    }
}

/*******************************************************************/

void UdpDaemon::stopServer()
{
    for(auto handler : qAsConst(d.handlers)) {
        handler->doDisconnect();
    }

    if (isListening()) {
        d.socket->close();
        d.socket->deleteLater();
        d.socket = Q_NULLPTR;
        QtServiceBase::instance()->logMessage("Sevice stopped");
    }
}

/*******************************************************************/

void UdpDaemon::pauseServer()
{
    d.disabled = true;
}

/*******************************************************************/

void UdpDaemon::resumeServer()
{
    d.disabled = false;
}

/*******************************************************************/

void UdpDaemon::readPendingDatagrams()
{
    while (d.socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = d.socket->receiveDatagram();
        QByteArray replyData = processData(datagram.data());
        if (!replyData.isEmpty()) {
            d.socket->writeDatagram(datagram.makeReply(replyData));
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
    d.handlers.append(fileHandler);

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
    d.handlers.append(dbHandler);

//    fileOptions.save(QSettings::SystemScope);
//    dbOptions.save(QSettings::SystemScope);
}

/*******************************************************************/

QByteArray UdpDaemon::processData(const QByteArray &data)
{
    QByteArray reply;
    // Handlers
    for(auto handler : qAsConst(d.handlers)) {
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
