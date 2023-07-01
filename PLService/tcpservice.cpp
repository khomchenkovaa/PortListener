#include "tcpservice.h"

/*******************************************************************/

TcpService::TcpService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "Qt TcpListener Daemon")
{
    setServiceDescription("A TcpListener service implemented with Qt");
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

/*******************************************************************/

void TcpService::start()
{
    QCoreApplication *app = application();
    const QStringList arguments = QCoreApplication::arguments();
    quint16 port = (arguments.size() > 1) ?
            arguments.at(1).toUShort() : 2424;
    daemon = new TcpDaemon(port, app);

    if (!daemon->isListening()) {
        logMessage(QString("Failed to bind to port %1").arg(daemon->serverPort()), QtServiceBase::Error);
        app->quit();
    }
}

/*******************************************************************/

void TcpService::pause()
{
    daemon->pauseServer();
}

/*******************************************************************/

void TcpService::resume()
{
    daemon->resumeServer();
}

/*******************************************************************/


