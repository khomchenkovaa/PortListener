#include "modbushandler.h"

#include <QTextStream>
#include <QDataStream>
#include <QBitArray>

#ifdef QT_GUI_LIB
#include <QTextBrowser>
#endif

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
    auto host    = QString("Coils from %1 size %2").arg(address).arg(size);

#ifdef QT_GUI_LIB
    auto logger  = qobject_cast<QTextBrowser*>(msg->logger);
#endif

    QStringList displayData;
    displayData << host;
    displayData << msg->timestamp.toString();
    for (const auto &item : qAsConst(d.doConf.items)) {
        int idx = item.ad - address;
        if (idx >=0 && idx < size) {
            displayData << QString("%1 : %2").arg(item.pin, data.testBit(idx) ? "1" : "0");
        }
    }

    clearErrors();

    QString output;
    QTextStream out(&output);
    for (const auto &str : qAsConst(displayData)) {
        out << str << Qt::endl;
#ifdef QT_GUI_LIB
        if (logger) logger->append(str);
#endif
    }

    if (isConnected() && d.outFile.isOpen()) {
        QTextStream fout(&d.outFile);
        fout << output;
    } else {
        addError(tr("Cannot write data to file"));
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
    auto host    = QString("Holding registers from %1 size %2").arg(address).arg(size);

#ifdef QT_GUI_LIB
    auto logger  = qobject_cast<QTextBrowser*>(msg->logger);
#endif

    QStringList displayData;
    displayData << host;
    displayData << msg->timestamp.toString();
    for (const auto &item : qAsConst(d.doConf.items)) {
        int idx = item.ad - address;
        if (idx >=0 && idx < size) {
            ModbusValue v;
            v.in.first = data.at(idx).toUInt();
            v.in.last  = data.at(idx+1).toUInt();
            displayData << QString("%1 [%2 %3]: %4")
                           .arg(item.pin)
                           .arg(v.in.first, 4, 16)
                           .arg(v.in.last, 4, 16)
                           .arg(v.out, 3, 'f');
        }
    }

    clearErrors();

    QString output;
    QTextStream out(&output);
    for (const auto &str : qAsConst(displayData)) {
        out << str << Qt::endl;
#ifdef QT_GUI_LIB
        if (logger) logger->append(str);
#endif
    }

    if (isConnected() && d.outFile.isOpen()) {
        QTextStream fout(&d.outFile);
        fout << output;
    } else {
        addError(tr("Cannot write data to file"));
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
