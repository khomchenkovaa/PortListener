#include "modbushandler.h"

#include <QTextStream>
#include <QDataStream>
#include <QBitArray>

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
    auto data    = msg->payload.toBitArray();

    QStringList displayData;
    displayData << msg->timestamp.toString();
    for (const auto &item : qAsConst(d.doConf.items)) {
        int idx = item.ad - address;
        if (idx >=0 && idx < size) {
            auto host = item.pin;
            auto info = data.testBit(idx) ? "1" : "0";
            displayData << QString("%1 : %2").arg(host, info);
            emit logMessage(host, info);
        }
    }

    QString output;
    QTextStream out(&output);
    for (const auto &str : qAsConst(displayData)) {
        out << str << Qt::endl;
    }

    if (isConnected() && d.outFile.isOpen()) {
        QTextStream fout(&d.outFile);
        fout << output;
    } else {
        emit logError(name(), tr("Cannot write data to file"));
    }
}

/********************************************************/

union ModbusValue {
    struct {
        quint16 last;
        quint16 first;
    } in;
    float out;
};

void ModbusHandler::processHoldingRegisters(PMessage msg)
{
    int  address = msg->headers.value("address").toInt();
    int  size    = msg->headers.value("size").toInt();
    auto data    = msg->payload.toList();

    QStringList displayData;
    displayData << msg->timestamp.toString();
    for (const auto &item : qAsConst(d.doConf.items)) {
        int idx = item.ad - address;
        if (idx >=0 && idx < size) {
            ModbusValue v;
            v.in.first = data.at(idx).toUInt();
            v.in.last  = data.at(idx+1).toUInt();
            auto host = QString("%1 [%2 %3]")
                    .arg(item.pin)
                    .arg(v.in.first, 4, 16)
                    .arg(v.in.last, 4, 16);
            auto info = QString::number(v.out, 'f', 3);
            displayData << QString("%1 : %2").arg(host, info);
            emit logMessage(host, info);
        }
    }

    QString output;
    QTextStream out(&output);
    for (const auto &str : qAsConst(displayData)) {
        out << str << Qt::endl;
    }

    if (isConnected() && d.outFile.isOpen()) {
        QTextStream fout(&d.outFile);
        fout << output;
    } else {
        emit logError(name(), tr("Cannot write data to file"));
    }
}

/********************************************************/

void ModbusHandler::doConnect(bool binary)
{
    clearErrors();
    const auto aoCsvFileName = settings()->value(Settings::AoCsvFileName).toString();
    if (aoCsvFileName.isEmpty()) {
        addError(tr("No AO CSV config file to open"));
    }
    d.aoConf.load(aoCsvFileName);

    const auto doCsvFileName = settings()->value(Settings::DoCsvFileName).toString();
    if (doCsvFileName.isEmpty()) {
        addError(tr("No DO CSV config file to open"));
    }
    d.doConf.load(doCsvFileName);

    const auto outFileName = settings()->value(Settings::OutFileName).toString();
    if (outFileName.isEmpty()) {
        addError(tr("No output file to open"));
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
        addError(tr("Could not open file"));
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
