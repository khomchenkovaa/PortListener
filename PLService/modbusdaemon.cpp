#include "modbusdaemon.h"

#include <QFileInfo>
#include <QDir>
#include <QTime>

ModbusDaemon::ModbusDaemon(QObject *parent)
    : QObject{parent}
{
    d.timer.setSingleShot(true);
    connect(&d.timer, &QTimer::timeout,
            this, &ModbusDaemon::doModbusRequest);

    connect(&d.modbusDevice, &QModbusClient::stateChanged,
            this, &ModbusDaemon::onStateChanged);
    connect(&d.modbusDevice, &QModbusClient::errorOccurred,
            this, &ModbusDaemon::handleDeviceError);

}

void ModbusDaemon::startServer()
{
    if (!loadOptions()) {
//        d.opt.save();
        qWarning("(Cannot read config file: %s)", QCoreApplication::applicationName().toLatin1().constData());
    }
    if (!loadSigConf()) {
        qWarning("(Cannot read sig file: %s)", MB_SIG_FILE);
    }
    d.timer.setInterval(d.opt.frequency * 1000);
    d.modbusDevice.setConnectionParameter(QModbusDevice::NetworkPortParameter, d.opt.port);
    d.modbusDevice.setConnectionParameter(QModbusDevice::NetworkAddressParameter, d.opt.host1);
    d.modbusDevice.setTimeout(d.opt.timeout);
    d.modbusDevice.setNumberOfRetries(d.opt.retries);
    if (!d.modbusDevice.connectDevice()) {
        qCritical("(Modbus client %s:%i connection error!\n%s)",
                 d.opt.host1.toLatin1().constData(),
                 d.opt.port,
                 d.modbusDevice.errorString().toLatin1().constData());
    } else {
        resumeServer();
    }
}

void ModbusDaemon::stopServer()
{
    pauseServer();
    d.modbusDevice.disconnectDevice();
}

void ModbusDaemon::pauseServer()
{
    d.disabled = true;
    d.timer.stop();
}

void ModbusDaemon::resumeServer()
{
    d.disabled = false;
    d.timer.start();
}

bool ModbusDaemon::loadSigConf()
{
    QFileInfo optFile(d.opt.optFilePath);
    QFileInfo sigFile(optFile.dir(), MB_SIG_FILE);
    if (sigFile.exists()) {
        d.conf.load(sigFile);
    }
    return !(d.conf.items.isEmpty());
}

void ModbusDaemon::onStateChanged()
{
    switch(d.modbusDevice.state()) {
    case QModbusDevice::UnconnectedState:
        qDebug("Modbus Device has Unconnected");
        break;
    case QModbusDevice::ConnectingState:
        qDebug("Modbus Device is Connecting");
        break;
    case QModbusDevice::ConnectedState:
        qDebug("Modbus Device has Connected");
        break;
    case QModbusDevice::ClosingState:
        qDebug("Modbus Device is Closing");
        break;
    }
}

void ModbusDaemon::handleDeviceError(QModbusDevice::Error newError)
{
    switch (newError) {
    case QModbusDevice::NoError:
        break;
    case QModbusDevice::ReadError:
        qWarning("Modbus Device read error: %s", d.modbusDevice.errorString().toLatin1().constData());
        break;
    case QModbusDevice::WriteError:
        qWarning("Modbus Device write error: %s", d.modbusDevice.errorString().toLatin1().constData());
        break;
    case QModbusDevice::ConnectionError:
        qWarning("Modbus Device connection error: %s", d.modbusDevice.errorString().toLatin1().constData());
        break;
    case QModbusDevice::ConfigurationError:
        qWarning("Modbus Device configuration error: %s", d.modbusDevice.errorString().toLatin1().constData());
        break;
    case QModbusDevice::TimeoutError:
        qWarning("Modbus Device timeout error: %s", d.modbusDevice.errorString().toLatin1().constData());
        break;
    case QModbusDevice::ProtocolError:
        qWarning("Modbus Device protocol error: %s", d.modbusDevice.errorString().toLatin1().constData());
        break;
    case QModbusDevice::ReplyAbortedError:
        qWarning("Modbus Device reply aborted error: %s", d.modbusDevice.errorString().toLatin1().constData());
        break;
    case QModbusDevice::UnknownError:
        qWarning("Modbus Device unknown error: %s", d.modbusDevice.errorString().toLatin1().constData());
        break;
    }
}

void ModbusDaemon::doModbusRequest()
{
    if (d.modbusDevice.state() == QModbusDevice::ConnectedState) {
        QString host = QString("%1:%2").arg(d.opt.host1).arg(d.opt.port);
        qDebug("Modbus Device Start request cycle: %s", host.toLatin1().constData());

        auto time = QTime::currentTime().msecsSinceStartOfDay() / 1000;
        qDebug("Sec from start of day: %i", time);

        for (const auto &item : qAsConst(d.conf.items)) {
            QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, item.ad, Modbus::dataTypeSizeOf(item.dt));
            if (auto *response = d.modbusDevice.sendReadRequest(request, d.opt.serverId)) {
                if (!response->isFinished())
                    connect(response, &QModbusReply::finished, this, [this, item](){
                        auto reply = qobject_cast<QModbusReply *>(sender());
                        if (!reply) return;

                        if (reply->error() == QModbusDevice::NoError) {
                            const QModbusDataUnit unit = reply->result();
                            switch(item.dt) {
                            case Modbus::RealType: {
                                auto arg = QVector<quint16>() << unit.value(0) << unit.value(1);
                                auto val = Modbus::takeFloat(arg);
                                auto info = QString::number(val, 'f', 3);
                                qDebug("Value: %s;%s", item.pin.toLatin1().constData(), info.toLatin1().constData());
                            } break;
                            case Modbus::DWordType: {
                                auto arg = QVector<quint16>() << unit.value(0) << unit.value(1);
                                auto val = Modbus::takeUInt(arg);
                                auto info = QString::number(val);
                                qDebug("Value: %s;%s", item.pin.toLatin1().constData(), info.toLatin1().constData());
                            } break;
                            case Modbus::IntType: {
                                quint16 val = unit.value(0);
                                auto info = QString::number(val);
                                qDebug("Value: %s;%s", item.pin.toLatin1().constData(), info.toLatin1().constData());
                            } break;
                            default:
                                break;
                            }
                        } else if (reply->error() == QModbusDevice::ProtocolError) {
                            qWarning("Modbus Device read response error: %s (Modbus exception: %i)",
                                   reply->errorString().toLatin1().constData(),
                                   reply->rawResult().exceptionCode());
                        } else {
                            qWarning("Modbus Device read response error: %s (code: %i)",
                                   reply->errorString().toLatin1().constData(),
                                   reply->error());
                        }

                        reply->deleteLater();
                    });
                else
                    delete response; // broadcast replies return immediately
            } else {
                qWarning("Modbus Device read register %s error: %s",
                       item.pin.toLatin1().constData(),
                       d.modbusDevice.errorString().toLatin1().constData());
            }
        }
    }

    d.timer.start(); // singleShort
}
