#include "modbusdaemon.h"

#include <QFileInfo>
#include <QDir>
#include <QTime>
#include <QTextStream>

ModbusDaemon::ModbusDaemon(QObject *parent)
    : QObject{parent}
{
    d.timer.setSingleShot(true);
    connect(&d.timer, &QTimer::timeout,
            this, &ModbusDaemon::doWork);

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
    prepareSensors();
    d.timer.setInterval(1000); // 1 sec timer
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
    d.conf.items.clear();
    d.sensors.clear();
}

void ModbusDaemon::pauseServer()
{
    d.disabled = true;
    d.timer.stop();
}

void ModbusDaemon::resumeServer()
{
    d.disabled = false;
    d.timerCounter = 0;
    d.responceSaved = true;
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

void ModbusDaemon::prepareSensors()
{
    for (const auto &item : qAsConst(d.conf.items)) {
        if (!d.sensors.contains(item.pin)) {
            d.sensors.insert(item.pin, PSensorValue::create());
            d.sensors[item.pin]->type = item.dt;
        }
        if (d.sensors[item.pin]->value.size() <= item.avg) {
            d.sensors[item.pin]->value.resize(item.avg + 1);
        }
    }
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

void ModbusDaemon::doWork()
{
    if (d.disabled) return;
    // FIXME async request
    if (d.timerCounter <= 0) {
        if (!d.responceSaved) {
            debugOutput();
            printOutput();
        }
        d.timerCounter  = d.opt.frequency;
        d.responceSaved = false;
        d.requestTime   = QDateTime::currentDateTimeUtc();
        // set all values as invalid
        QMapIterator<QString, PSensorValue> i(d.sensors);
        while (i.hasNext()) {
            i.next();
            i.value()->clear();
        }
        // request sensor's values
        doModbusRequest();
    }

    d.timerCounter--;

    if (!d.responceSaved) {
        if (isResponseValid()) {
            debugOutput();
            printOutput();
            d.responceSaved = true;
        }
    }
    d.timer.start();
}

void ModbusDaemon::doModbusRequest()
{
    if (d.modbusDevice.state() == QModbusDevice::ConnectedState) {
        QString host = QString("%1:%2").arg(d.opt.host1).arg(d.opt.port);
        qDebug("Modbus Device Start request cycle: %s", host.toLatin1().constData());

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
                                d.sensors[item.pin]->setValue(item.avg, val);
                            } break;
                            case Modbus::DWordType: {
                                auto arg = QVector<quint16>() << unit.value(0) << unit.value(1);
                                auto val = Modbus::takeUInt(arg);
                                d.sensors[item.pin]->setValue(item.avg, val);
                            } break;
                            case Modbus::IntType: {
                                auto val = unit.value(0);
                                d.sensors[item.pin]->setValue(item.avg, val);
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

bool ModbusDaemon::isResponseValid() const
{
    QMapIterator<QString, PSensorValue> i(d.sensors);
    while (i.hasNext()) {
        i.next();
        if (!i.value()->isValid()) return false;
    }
    return true;
}

void ModbusDaemon::printOutput()
{
    int sec = d.requestTime.time().msecsSinceStartOfDay() / 1000;
    QString fileName = outputFileName();
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning("Cannot open file %s to write\n", fileName.toLatin1().constData());
        debugOutput();
        return;
    }

    QTextStream out(&file);
    out << sec << Qt::endl;
    QMapIterator<QString, PSensorValue> i(d.sensors);
    while (i.hasNext()) {
        i.next();
        if (i.value()->isValid()) {
            out << QString("%1; %2").arg(i.key(), i.value()->valAsString()) << Qt::endl;
        }
    }
    out.flush();
    file.close();
}

void ModbusDaemon::debugOutput()
{
    int sec = d.requestTime.time().msecsSinceStartOfDay() / 1000;
    qDebug("Sec from start of day: \n%i", sec);
    QMapIterator<QString, PSensorValue> i(d.sensors);
    while (i.hasNext()) {
        i.next();
        if (i.value()->isValid()) {
            qDebug("%s; %s", i.key().toLatin1().constData(), i.value()->valAsString().toLatin1().constData());
        } else {
            qDebug("%s; %s", i.key().toLatin1().constData(), "invalid");
        }
    }
}

QString ModbusDaemon::outputFileName() const
{
    return QString("%1/%2_%3").arg(d.opt.outputDir, d.opt.unit, d.requestTime.toString("yyyy_MM_dd"));
}
