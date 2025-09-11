#ifndef MODBUSDAEMON_H
#define MODBUSDAEMON_H

#include "modbusoptions.h"
#include "modbusclientconf.h"
#include "qtservice.h"

#include <QModbusTcpClient>
#include <QTimer>

class ModbusDaemon : public QObject
{
    Q_OBJECT

    struct ModbusDaemonPrivate {
        bool        disabled = false;
        QModbusTcpClient modbusDevice;
        Settings::ModbusOptions opt;
        ModbusSigConf    conf;
        QTimer           timer;
    };

public:
    explicit ModbusDaemon(QObject *parent = nullptr);

public slots:
    void startServer();
    void stopServer();
    void pauseServer();
    void resumeServer();

signals:

private Q_SLOTS:
    bool loadOptions() {
        d.opt.load();
        return !(d.opt.optFilePath.isEmpty());
    }

    bool loadSigConf();
    void onStateChanged();
    void handleDeviceError(QModbusDevice::Error newError);
    void doModbusRequest();

private:
    ModbusDaemonPrivate d;
};

#endif // MODBUSDAEMON_H
