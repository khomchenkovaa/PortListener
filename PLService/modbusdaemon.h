#ifndef MODBUSDAEMON_H
#define MODBUSDAEMON_H

#include "modbusoptions.h"
#include "modbusclientconf.h"
#include "qtservice.h"

#include <QModbusTcpClient>
#include <QTimer>
#include <QSharedPointer>

struct SensorValue {
    bool    valid = false;
    Modbus::DataType type = Modbus::UnkownType;
    QVariant value;
    QVector<QVariant> input;

    bool needCalc() {
        return input.size() > 0;
    }

    void setValue(int idx, QVariant val) {
        valid = true;
        if (needCalc()) {
            if (idx > 0 && input.size() >= idx) {
                input[idx-1] = val;
            }
        } else {
            value = val;
        }
    }

    void doCalc() {
        if (!needCalc()) return;
        switch(type) {
        case Modbus::RealType: {
            float sum = 0;
            for (auto i = input.begin(); i < input.end(); ++i) {
                sum += i->toFloat();
            }
            value = sum / input.size();
        } break;
        case Modbus::DWordType: {
            quint32 sum = 0;
            for (auto i = input.begin(); i < input.end(); ++i) {
                sum += i->toUInt();
            }
            value = sum / input.size();
        } break;
        case Modbus::IntType: {
            quint16 sum = 0;
            for (auto i = input.begin(); i < input.end(); ++i) {
                sum += i->toUInt();
            }
            value = sum / input.size();
        } break;
        default:
            break;
        }
    }

    QString valAsString() {
        if (Modbus::RealType == type) {
            return QString::number(value.toFloat(), 'f', 3);
        }
        return QString::number(value.toUInt());
    }
};

typedef QSharedPointer<SensorValue> PSensorValue;

class ModbusDaemon : public QObject
{
    Q_OBJECT

    struct ModbusDaemonPrivate {
        bool disabled = false;
        QModbusTcpClient modbusDevice;
        Settings::ModbusOptions opt;
        ModbusSigConf    conf;
        QTimer           timer;
        QMap<QString, PSensorValue> sensors;
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
    void prepareSensors();
    void onStateChanged();
    void handleDeviceError(QModbusDevice::Error newError);
    void doWork();
    void doModbusRequest();

private:
    ModbusDaemonPrivate d;
};

#endif // MODBUSDAEMON_H
