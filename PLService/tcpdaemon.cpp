#include "tcpdaemon.h"

#include "filehandler.h"

/*******************************************************************/

TcpDaemon::TcpDaemon(quint16 port, QObject* parent):
    QTcpServer(parent), disabled(false), m_Handler(Q_NULLPTR)
{
    initHandler();
    startServer(port);
}

/*******************************************************************/

TcpDaemon::~TcpDaemon() {
    stopServer();
}

/*******************************************************************/

void TcpDaemon::startServer(quint16 port) {
    m_Handler->doConnect();
	if(!isListening()) {
        if(listen(QHostAddress::Any, port)) {
            QtServiceBase::instance()->logMessage("Sevice started");
        } else {
            QtServiceBase::instance()->logMessage(tr("Sevice startup error: %1").arg(errorString()));
        }
    } else {
        QtServiceBase::instance()->logMessage("Sevice already started");
    }
}

/*******************************************************************/

void TcpDaemon::stopServer() {
    m_Handler->doDisconnect();
    if(isListening()) {
		close();
        QtServiceBase::instance()->logMessage("Sevice stopped");
    }
}

/*******************************************************************/

void TcpDaemon::pauseServer()
{
    disabled = true;
}

/*******************************************************************/

void TcpDaemon::resumeServer()
{
    disabled = false;
}

/*******************************************************************/

void TcpDaemon::incomingConnection(qintptr handle) {
    if (disabled) return;

	QTcpSocket* socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead,
            this, &TcpDaemon::socketRead);
    connect(socket, &QTcpSocket::stateChanged,
            this, &TcpDaemon::socketStateChanged);
    socket->setSocketDescriptor(handle);

    QtServiceBase::instance()->logMessage("New Connection");
}

/*******************************************************************/

void TcpDaemon::socketRead() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if(socket) {
        QByteArray data = socket->readAll();
        QByteArray replyData = processData(data);
        if (!replyData.isEmpty()) {
            socket->write(replyData);
        }
    }
}

/*******************************************************************/

void TcpDaemon::socketStateChanged(QAbstractSocket::SocketState socketState) {
    if (socketState == QAbstractSocket::UnconnectedState) {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        discardClient(socket);
    }
}

/********************************************************/

void TcpDaemon::discardClient(QTcpSocket *socket)
{
    if (!socket) return;
    socket->deleteLater();
    QtServiceBase::instance()->logMessage("Connection closed");
}

/********************************************************/

void TcpDaemon::initHandler()
{
    m_Handler = new FileHandler(this);
    SettingsMap map;
    map.insert(FileHandler::FileName, "/home/khoman/tcp_log.txt");
    map.insert(FileHandler::FileAppend, true);
    m_Handler->setSettings(map);
}

/********************************************************/

QByteArray TcpDaemon::processData(const QByteArray &data)
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
