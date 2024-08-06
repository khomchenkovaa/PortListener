#include "gatehandler.h"

#include <QTextStream>
#include <QDataStream>

/********************************************************/

GateHandler::GateHandler(QObject *parent)
    : MessageHandler(tr("Gate handler"), parent)
{

}

/********************************************************/

GateHandler::~GateHandler()
{
    GateHandler::doDisconnect();
}

/********************************************************/

QByteArray GateHandler::processData(const QByteArray &data)
{
    clearErrors();

    if (data.size() != d.defConf.size()) {
        addError(tr("Wrong size of the incomming packet"));
    }

    QString output;
    QTextStream out(&output);

    for (int i=0; i<d.defConf.fields(); ++i) {
        if (d.defConf.type(i).compare("type_timeval") == 0) {
            const QByteArray ba = data.mid(d.defConf.offset(i), 8);
            QDataStream ds(ba);
            int time = 0;
            ds >> time;
            out << time << Qt::endl;
        }
        if (d.defConf.type(i).compare("type_ival") == 0) {
            const QByteArray ba = data.mid(d.defConf.offset(i), 8);
            QDataStream ds(ba);
            qint64 value = 0;
            ds >> value;
            out << d.defConf.name(i) << " " << value << Qt::endl;
        }
        if (d.defConf.type(i).compare("type_rval") == 0) {
            const QByteArray ba = data.mid(d.defConf.offset(i), 8);
            QDataStream ds(ba);
            qreal value = 0.0;
            ds >> value;
            out << d.defConf.name(i) << " " << value << Qt::endl;
        }
    }

    if (isConnected() && d.outFile.isOpen()) {
        QTextStream fout(&d.outFile);
        fout << output;
    } else {
        addError(tr("Cannot write data to file"));
    }
    return MessageHandler::processData(data);
}

/********************************************************/

void GateHandler::doConnect(bool binary)
{
    clearErrors();
    const auto csvFileName = settings()->value(Settings::CsvFileName).toString();
    if (csvFileName.isEmpty()) {
        addError(tr("No CSV config file to open"));
    }
    d.csvConf.load(csvFileName);

    const auto defFileName = settings()->value(Settings::DefFileName).toString();
    if (defFileName.isEmpty()) {
        addError(tr("No DEF config file to open"));
    }
    d.defConf.load(defFileName);

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

void GateHandler::doDisconnect()
{
    if (d.outFile.isOpen()) {
        d.outFile.close();
    }
    setDisconnected();
}

/********************************************************/
