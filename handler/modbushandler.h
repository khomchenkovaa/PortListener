#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include "messagehandler.h"
#include "modbushandlerconf.h"

#include <QFile>

class ModbusHandler : public MessageHandler
{
    Q_OBJECT

    struct ModbusHandlerData {
        ModbusCsvConf aoConf; ///< analog output (holding registers)
        ModbusCsvConf doConf; ///< digital output (coins)
        QFile         outFile;
        int           outSec = -1;
        QVariantMap   allValues;
        QVariantMap   outValues;
    };
public:
    enum Settings {
        AoCsvFileName,
        DoCsvFileName,
        OutFileName,
        FileAppend
    };

    explicit ModbusHandler(QObject *parent = nullptr);

    ~ModbusHandler();

    QByteArray handleMessage(PMessage msg);
    void processCoils(PMessage msg);
    void processHoldingRegisters(PMessage msg);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    void addOutValue(const QString &kks, const QString &value);
    void printOutValues(int sec);

private:
    ModbusHandlerData d;
};

#endif // MODBUSHANDLER_H
