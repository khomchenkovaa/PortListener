#ifndef MODBUSHANDLER_H
#define MODBUSHANDLER_H

#include "messagehandler.h"
#include "modbushandlerconf.h"

#include <QFile>

class ModbusHandler : public MessageHandler
{
    Q_OBJECT

    struct ModbusHandlerPrivate {
        ModbusCsvConf aoConf; ///< analog output (holding registers)
        ModbusCsvConf doConf; ///< digital output (coins)
        QFile         outFile;
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
    QByteArray processData(const QByteArray& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    ModbusHandlerPrivate d;
};

#endif // MODBUSHANDLER_H
