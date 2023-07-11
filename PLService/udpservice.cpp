#include "udpservice.h"

/*******************************************************************/

UdpService::UdpService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "Qt UdpListener Daemon")
{
    setServiceDescription("A UdpListener service implemented with Qt");
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

/*******************************************************************/

void UdpService::start()
{
    QCoreApplication *app = application();
    const QStringList arguments = QCoreApplication::arguments();
    quint16 port = (arguments.size() > 1) ?
            arguments.at(1).toUShort() : 2424;
    daemon = new UdpDaemon(port, app);

    if (!daemon->isListening()) {
        logMessage(QString("Failed to bind to port %1").arg(port), QtServiceBase::Error);
        app->quit();
    }
}

/*******************************************************************/

void UdpService::pause()
{
    daemon->pauseServer();
}

/*******************************************************************/

void UdpService::resume()
{
    daemon->resumeServer();
}

/*******************************************************************/


