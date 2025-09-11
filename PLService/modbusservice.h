#ifndef MODBUSSERVICE_H
#define MODBUSSERVICE_H

#include "modbusdaemon.h"

#include <QCoreApplication>

class ModbusService : public QtService<QCoreApplication>
{
public:
    ModbusService(int argc, char **argv)
        : QtService<QCoreApplication>(argc, argv, "Modbus-TCP Client Daemon") {
        setServiceDescription("A Modbus-TCP Client service implemented with Qt");
        setServiceFlags(QtServiceBase::CanBeSuspended);
    }

protected:
    void start() {
        QCoreApplication *app = application();
        daemon = new ModbusDaemon(app);
        daemon->startServer();
    }

    void stop() {
        if (daemon) {
            daemon->stopServer();
            daemon->deleteLater();
            daemon = Q_NULLPTR;
        }
    }

    void pause() {
        daemon->pauseServer();
    }

    void resume() {
        daemon->resumeServer();
    }

private:
    ModbusDaemon *daemon = Q_NULLPTR;
};

#endif // MODBUSSERVICE_H
