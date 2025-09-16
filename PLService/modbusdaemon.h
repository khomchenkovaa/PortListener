#ifndef MODBUSDAEMON_H
#define MODBUSDAEMON_H

#include "modbusoptions.h"
#include "modbusclientconf.h"
#include "qtservice.h"

#include <QModbusTcpClient>
#include <QTimer>
#include <QDateTime>
#include <QSharedPointer>

struct SensorValue {
    Modbus::DataType type = Modbus::UnkownType;
    QVector<QVariant> value;

    bool isValid() const {
        if (value.isEmpty()) return false;
        if (value.size() == 1) {
            return value.constFirst().isValid();
        }
        for (int i = 1; i < value.size(); ++i) {
            if (!value[i].isValid()) return false;
        }
        return true;
    }

    void clear() {
        for (int i = 0; i < value.size(); ++i) {
            value[i].clear();
        }
    }

    bool needCalc() {
        return value.size() > 1;
    }

    void setValue(int idx, QVariant val) {
        if (idx >=0 && idx < value.size()) {
            value[idx] = val;
        }
    }

    void doCalc() {
        auto i = value.begin();
        ++i; // skip first
        switch(type) {
        case Modbus::RealType: {
            float sum = 0;
            while (i != value.end()) {
                sum += i->toFloat();
                ++i;
            }
            value[0] = sum / value.size();
        } break;
        case Modbus::DWordType: {
            quint32 sum = 0;
            while (i != value.end()) {
                sum += i->toUInt();
                ++i;
            }
            value[0] = sum / value.size();
        } break;
        case Modbus::IntType: {
            quint16 sum = 0;
            while (i != value.end()) {
                sum += i->toUInt();
                ++i;
            }
            value[0] = sum / value.size();
        } break;
        default:
            break;
        }
    }

    QString valAsString() {
        // calc averages if needed
        if (needCalc()) doCalc();

        if (Modbus::RealType == type) {
            return QString::number(value.constFirst().toFloat(), 'f', 3);
        }
        return QString::number(value.constFirst().toUInt());
    }
};

typedef QSharedPointer<SensorValue> PSensorValue;

class ModbusDaemon : public QObject
{
    Q_OBJECT

    struct ModbusDaemonPrivate {
        bool disabled = false;
        QModbusTcpClient modbusDevice1;
        QModbusTcpClient modbusDevice2;
        Settings::ModbusOptions opt;
        ModbusSigConf    conf;
        QMap<QString, PSensorValue> sensors;
        QTimer timer;
        QDateTime requestTime = QDateTime::currentDateTimeUtc();
        int    timerCounter = 0;
        bool   responceSaved = true;
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
    void onStateChanged();
    void handleDeviceError(QModbusDevice::Error newError);
    void doWork();

private:
    bool loadOptions() {
        d.opt.load();
        return !(d.opt.optFilePath.isEmpty());
    }

    bool loadSigConf();
    void prepareSensors();
    void doModbusRequest();
    bool isResponseValid() const;
    void printOutput();
    void debugOutput();
    QString outputFileName() const;
    void checkAndConnect();

private:
    ModbusDaemonPrivate d;
};

#endif // MODBUSDAEMON_H
