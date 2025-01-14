#include "modbushandler.h"

#include "modbushelper.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QDataStream>
#include <QBitArray>
#include <QFileInfo>
#include <QDir>

/********************************************************/

ModbusHandler::ModbusHandler(QObject *parent)
    : MessageHandler(tr("Modbus handler"), parent)
{

}

/********************************************************/

ModbusHandler::~ModbusHandler()
{
    ModbusHandler::doDisconnect();
}

/********************************************************/

QByteArray ModbusHandler::handleMessage(PMessage msg)
{
    switch (msg->payloadType) {
    case QMetaType::QBitArray:
        processCoils(msg);
        break;
    case QMetaType::QVariantList:
        processHoldingRegisters(msg);
        break;
    default:
        break;
    }
    return QByteArray();
}

/********************************************************/

void ModbusHandler::processCoils(PMessage msg)
{
    int  address = msg->headers.value("address").toInt();
    int  size    = msg->headers.value("size").toInt();
    int  srv     = msg->headers.value("srv").toInt();
    auto data    = msg->payload.toBitArray();

    for (const auto &item : qAsConst(d.doConf.items)) {
        if (srv != item.dn) continue; // only our server
        if (!item.iot) continue;      // only input
        int idx = item.ad - address;
        if (idx >=0 && idx < size) {
            auto host = item.pin;
            auto info = data.testBit(idx) ? "1" : "0";
            addOutValue(host, info);
            emit logMessage(host, info);
        }
    }

    printOutValues(msg->timestamp.time().msecsSinceStartOfDay() / 1000);
}

/********************************************************/

void ModbusHandler::processHoldingRegisters(PMessage msg)
{
    int  address = msg->headers.value("address").toInt();
    int  size    = msg->headers.value("size").toInt();
    int  srv     = msg->headers.value("srv").toInt();
    auto data    = msg->payload.toList();

    for (const auto &item : qAsConst(d.aoConf.items)) {
        if (srv != item.dn) continue; // only our server
        if (!item.iot) continue;      // only input
        int idx = item.ad - address;
        if (idx >=0 && idx < size) {
            switch(item.dataType()) {
            case Modbus::RealType: {
                Modbus::ModbusValue v;
                v.in.first = data.at(idx).toUInt();
                v.in.last  = data.at(idx+1).toUInt();
                auto host = QString("%1 [%2 %3]")
                        .arg(item.pin)
                        .arg(v.in.first, 4, 16, QLatin1Char('0'))
                        .arg(v.in.last, 4, 16, QLatin1Char('0'));
                auto info = QString::number(v.outFloat, 'f', 3);
                addOutValue(item.pin, info);
                emit logMessage(host, info);
            } break;
            case Modbus::DWordType: {
                Modbus::ModbusValue v;
                v.in.first = data.at(idx).toUInt();
                v.in.last  = data.at(idx+1).toUInt();
                auto host = QString("%1 [%2 %3]")
                        .arg(item.pin)
                        .arg(v.in.first, 4, 16, QLatin1Char('0'))
                        .arg(v.in.last, 4, 16, QLatin1Char('0'));
                auto info = QString::number(v.outUInt);
                addOutValue(item.pin, info);
                emit logMessage(host, info);
            } break;
            case Modbus::IntType: {
                quint16 val = data.at(idx).toUInt();
                auto host = QString("%1 [%2]")
                        .arg(item.pin)
                        .arg(val, 4, 16, QLatin1Char('0'));
                auto info = QString::number(val);
                addOutValue(item.pin, info);
                emit logMessage(host, info);
            } break;
            default:
                break;
            }
        }
    }

    printOutValues(msg->timestamp.time().msecsSinceStartOfDay() / 1000);
}

/********************************************************/

void ModbusHandler::doConnect(bool binary)
{
    clearErrors();
    const auto aoCsvFileName = settings()->value(Settings::AoCsvFileName).toString();
    if (aoCsvFileName.isEmpty()) {
        addError(tr("No AO CSV config file is present"));
    }
    d.aoConf.load(aoCsvFileName, false);

    auto doCsvFileName = settings()->value(Settings::DoCsvFileName).toString();
    if (doCsvFileName.isEmpty()) {
        addError(tr("No DO CSV config file is present. Use AO CSV config file"));
        doCsvFileName = aoCsvFileName;
    }
    if (doCsvFileName.isEmpty()) {
        addError(tr("No DO CSV config file to open"));
    }
    d.doConf.load(doCsvFileName, true);

    auto outFileName = settings()->value(Settings::OutFileName).toString();
    if (outFileName.isEmpty()) {
        const auto fileName = QString("prefix%1.txt").arg(QDate::currentDate().toString("_yyyy_MM_dd"));
        const QFileInfo fileInfo(QDir(QCoreApplication::applicationDirPath()), fileName);
        outFileName = fileInfo.absoluteFilePath();
        addError(tr("No output file is present. Using %1 as output file").arg(fileName));
    }
    d.outFile.setFileName(outFileName);
    QIODevice::OpenMode flags;
    if (settings()->value(Settings::FileAppend, true).toBool()) {
        flags = QIODevice::Append;
    } else {
        flags = QIODevice::WriteOnly;
    }
    if (!binary) {
        flags |= QIODevice::Text;
    }
    if (!d.outFile.open(flags)) {
        addError(tr("Could not open output file"));
    }
    setConnected();
}

/********************************************************/

void ModbusHandler::doDisconnect()
{
    if (d.outFile.isOpen()) {
        d.outFile.close();
    }
    setDisconnected();
}

/********************************************************/

void ModbusHandler::addOutValue(const QString &kks, const QString &value)
{
    if (d.allValues.value(kks).toString() != value) {
        d.outValues.insert(kks, value);
        d.allValues.insert(kks, value);
    }
}

/********************************************************/

void ModbusHandler::printOutValues(int sec)
{
    if (d.outValues.isEmpty()) return;

    QString output;
    QTextStream out(&output);

    if (d.outSec != sec) {
        d.outSec = sec;
        out << QString::number(d.outSec) << Qt::endl;
    }

    for (auto i = d.outValues.constBegin(); i != d.outValues.constEnd(); i++) {
        out << i.key() << ";" << i.value().toString() << Qt::endl;
    }

    d.outValues.clear();

    if (isConnected() && d.outFile.isOpen()) {
        QTextStream fout(&d.outFile);
        fout << output;
    } else {
        emit logError(name(), tr("Cannot write data to file"));
    }
}

/********************************************************/
