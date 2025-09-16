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

    connect(&d.modbusDevice1, &QModbusClient::stateChanged,
            this, &ModbusDaemon::onStateChanged);
    connect(&d.modbusDevice1, &QModbusClient::errorOccurred,
            this, &ModbusDaemon::handleDeviceError);
    connect(&d.modbusDevice2, &QModbusClient::stateChanged,
            this, &ModbusDaemon::onStateChanged);
    connect(&d.modbusDevice2, &QModbusClient::errorOccurred,
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
    // prepare modbus device 1
    d.modbusDevice1.setConnectionParameter(QModbusDevice::NetworkPortParameter, d.opt.port);
    d.modbusDevice1.setConnectionParameter(QModbusDevice::NetworkAddressParameter, d.opt.host1);
    d.modbusDevice1.setTimeout(d.opt.timeout);
    d.modbusDevice1.setNumberOfRetries(d.opt.retries);
    // prepare modbus device 2
    d.modbusDevice2.setConnectionParameter(QModbusDevice::NetworkPortParameter, d.opt.port);
    d.modbusDevice2.setConnectionParameter(QModbusDevice::NetworkAddressParameter, d.opt.host2);
    d.modbusDevice2.setTimeout(d.opt.timeout);
    d.modbusDevice2.setNumberOfRetries(d.opt.retries);

    resumeServer();
}

void ModbusDaemon::stopServer()
{
    pauseServer();
    d.conf.items.clear();
    d.sensors.clear();
}

void ModbusDaemon::pauseServer()
{
    d.disabled = true;
    d.timer.stop();
    if (d.modbusDevice1.state() != QModbusDevice::UnconnectedState) {
        d.modbusDevice1.disconnectDevice();
    }
    if (d.modbusDevice2.state() != QModbusDevice::UnconnectedState) {
        d.modbusDevice2.disconnectDevice();
    }
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
    QModbusTcpClient *s = qobject_cast<QModbusTcpClient *>(sender());
    if (s == Q_NULLPTR) return;
    int deviceNum = (s == &d.modbusDevice1) ? 1 : 2;
    switch(s->state()) {
    case QModbusDevice::UnconnectedState:
        qDebug("Modbus Device %i has Unconnected", deviceNum);
        break;
    case QModbusDevice::ConnectingState:
        qDebug("Modbus Device %i is Connecting", deviceNum);
        break;
    case QModbusDevice::ConnectedState:
        qDebug("Modbus Device %i has Connected", deviceNum);
        break;
    case QModbusDevice::ClosingState:
        qDebug("Modbus Device %i is Closing", deviceNum);
        break;
    }
}

void ModbusDaemon::handleDeviceError(QModbusDevice::Error newError)
{
    QModbusTcpClient *s = qobject_cast<QModbusTcpClient *>(sender());
    if (s == Q_NULLPTR) return;
    int deviceNum = (s == &d.modbusDevice1) ? 1 : 2;
    QString errorString = (s == &d.modbusDevice1) ? d.modbusDevice1.errorString() : d.modbusDevice2.errorString();
    switch (newError) {
    case QModbusDevice::NoError:
        break;
    case QModbusDevice::ReadError:
        qWarning("Modbus Device %i read error: %s", deviceNum, qPrintable(errorString));
        break;
    case QModbusDevice::WriteError:
        qWarning("Modbus Device %i write error: %s", deviceNum, qPrintable(errorString));
        break;
    case QModbusDevice::ConnectionError:
        qWarning("Modbus Device %i connection error: %s", deviceNum, qPrintable(errorString));
        break;
    case QModbusDevice::ConfigurationError:
        qWarning("Modbus Device %i configuration error: %s", deviceNum, qPrintable(errorString));
        break;
    case QModbusDevice::TimeoutError:
        qWarning("Modbus Device %i timeout error: %s", deviceNum, qPrintable(errorString));
        break;
    case QModbusDevice::ProtocolError:
        qWarning("Modbus Device %i protocol error: %s", deviceNum, qPrintable(errorString));
        break;
    case QModbusDevice::ReplyAbortedError:
        qWarning("Modbus Device %i reply aborted error: %s", deviceNum, qPrintable(errorString));
        break;
    case QModbusDevice::UnknownError:
        qWarning("Modbus Device %i unknown error: %s", deviceNum, qPrintable(errorString));
        break;
    }
}

void ModbusDaemon::doWork()
{
    if (d.disabled) return;
    // async request
    checkAndConnect();

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
    QModbusTcpClient *s = Q_NULLPTR;
    int deviceNum = 0;
    QString host;
    if (d.modbusDevice1.state() == QModbusDevice::ConnectedState) {
        deviceNum = 1;
        host = QString("%1:%2").arg(d.opt.host1).arg(d.opt.port);
        s = &d.modbusDevice1;
    } else if (d.modbusDevice2.state() == QModbusDevice::ConnectedState) {
        deviceNum = 2;
        host = QString("%1:%2").arg(d.opt.host2).arg(d.opt.port);
        s = &d.modbusDevice2;
    }

    if (s != Q_NULLPTR) {
        qDebug("Modbus Device %i Start request cycle: %s", deviceNum, qPrintable(host));

        for (const auto &item : qAsConst(d.conf.items)) {
            QModbusDataUnit request(QModbusDataUnit::HoldingRegisters, item.ad, Modbus::dataTypeSizeOf(item.dt));
            if (auto *response = s->sendReadRequest(request, d.opt.serverId)) {
                if (!response->isFinished())
                    connect(response, &QModbusReply::finished, this, [this, item, deviceNum](){
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
                            qWarning("Modbus Device %i read response error: %s (Modbus exception: %i)",
                                     deviceNum,
                                     reply->errorString().toLatin1().constData(),
                                     reply->rawResult().exceptionCode());
                        } else {
                            qWarning("Modbus Device %i read response error: %s (code: %i)",
                                     deviceNum,
                                     reply->errorString().toLatin1().constData(),
                                     reply->error());
                        }

                        reply->deleteLater();
                    });
                else
                    delete response; // broadcast replies return immediately
            } else {
                qWarning("Modbus Device %i read register %s error: %s",
                         deviceNum,
                         item.pin.toLatin1().constData(),
                         s->errorString().toLatin1().constData());
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

void ModbusDaemon::checkAndConnect()
{
    if (d.modbusDevice1.state() == QModbusDevice::UnconnectedState) {
        if (!d.modbusDevice1.connectDevice()) {
            qCritical("(Modbus client 1 (%s:%i) connection error!\n%s)",
                     d.opt.host1.toLatin1().constData(),
                     d.opt.port,
                     d.modbusDevice1.errorString().toLatin1().constData());
        }
    }
    if (d.modbusDevice2.state() == QModbusDevice::UnconnectedState) {
        if (!d.modbusDevice2.connectDevice()) {
            qCritical("(Modbus client 2 (%s:%i) connection error!\n%s)",
                     d.opt.host2.toLatin1().constData(),
                     d.opt.port,
                     d.modbusDevice2.errorString().toLatin1().constData());
        }
    }
}
