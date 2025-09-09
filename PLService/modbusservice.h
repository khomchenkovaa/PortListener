#ifndef MODBUSSERVICE_H
#define MODBUSSERVICE_H

#include "modbusdaemon.h"

#include <QCoreApplication>

class ModbusService : public QtService<QCoreApplication>
{
public:
    ModbusService(int argc, char **argv);

protected:
    void start();
    void stop();
    void pause();
    void resume();

private:
    ModbusDaemon *daemon;
};

#endif // MODBUSSERVICE_H
