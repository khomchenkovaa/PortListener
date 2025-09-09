#include "modbusservice.h"

#include <QSettings>

ModbusService::ModbusService(int argc, char **argv)
    : QtService<QCoreApplication>(argc, argv, "Modbus-TCP Client Daemon")
{
    setServiceDescription("A Modbus-TCP Client service implemented with Qt");
    setServiceFlags(QtServiceBase::CanBeSuspended);
    daemon = Q_NULLPTR;
}

void ModbusService::start()
{
    QCoreApplication *app = application();
    daemon = new ModbusDaemon(app);
    daemon->startServer();
}

void ModbusService::stop()
{
    if (daemon) {
        daemon->stopServer();
        daemon->deleteLater();
        daemon = Q_NULLPTR;
    }
}

void ModbusService::pause()
{
    daemon->pauseServer();
}

void ModbusService::resume()
{
    daemon->resumeServer();
}
