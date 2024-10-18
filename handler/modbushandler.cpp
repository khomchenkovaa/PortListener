#include "modbushandler.h"

#include <QTextStream>
#include <QDataStream>

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

QByteArray ModbusHandler::processData(const QByteArray &data)
{
    clearErrors();

//    if (data.size() != d.defConf.size()) {
//        addError(tr("Wrong size of the incomming packet"));
//    }

    QString output;
    QTextStream out(&output);

//    for (int i=0; i<d.defConf.fields(); ++i) {
//        if (d.defConf.type(i).compare("type_timeval") == 0) {
//            const QByteArray ba = data.mid(d.defConf.offset(i), 8);
//            QDataStream ds(ba);
//            int time = 0;
//            ds >> time;
//            out << time << Qt::endl;
//        }
//        if (d.defConf.type(i).compare("type_ival") == 0) {
//            const QByteArray ba = data.mid(d.defConf.offset(i), 8);
//            QDataStream ds(ba);
//            qint64 value = 0;
//            ds >> value;
//            out << d.defConf.name(i) << " " << value << Qt::endl;
//        }
//        if (d.defConf.type(i).compare("type_rval") == 0) {
//            const QByteArray ba = data.mid(d.defConf.offset(i), 8);
//            QDataStream ds(ba);
//            qreal value = 0.0;
//            ds >> value;
//            out << d.defConf.name(i) << " " << value << Qt::endl;
//        }
//    }

    if (isConnected() && d.outFile.isOpen()) {
        QTextStream fout(&d.outFile);
        fout << output;
    } else {
        addError(tr("Cannot write data to file"));
    }
    return MessageHandler::processData(data);
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
