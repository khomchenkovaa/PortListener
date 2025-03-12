#include "dephandler.h"

#include <QTextStream>
#include <QDataStream>

/********************************************************/

DepHandler::DepHandler(QObject *parent)
    : MessageHandler(tr("Gate handler"), parent)
{

}

/********************************************************/

DepHandler::~DepHandler()
{
    DepHandler::doDisconnect();
}

/********************************************************/

QByteArray DepHandler::processData(const QByteArray &data)
{
    clearErrors();

    QByteArray buff(data);
    if (d.defConf.size() > data.size()) {
        buff.append(d.defConf.size() - data.size(), '\0');
    }

    QString output;
    QTextStream out(&output);

    for (int i=0; i<d.defConf.fields(); ++i) {
        switch(d.defConf.typeId(i)) {
        case Gate::TIMEVAL: { // type_timeval
            const QByteArray ba = buff.mid(d.defConf.offset(i), 8);
            QDataStream ds(ba);
            int time = 0;
            ds >> time;
            out << time << Qt::endl;
        } break;
        case Gate::FLOATVALID: { // type_rval
            const QByteArray ba = buff.mid(d.defConf.offset(i), 8);
            QDataStream ds(ba);
            qreal value = 0.0;
            ds >> value;
            out << d.defConf.name(i) << " " << value << Qt::endl;
        } break;
        case Gate::SDWORDVALID: { // type_ival
            const QByteArray ba = buff.mid(d.defConf.offset(i), 8);
            QDataStream ds(ba);
            qint64 value = 0;
            ds >> value;
            out << d.defConf.name(i) << " " << value << Qt::endl;
        } break;
        default:
            break;
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

void DepHandler::doConnect(bool binary)
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

void DepHandler::doDisconnect()
{
    if (d.outFile.isOpen()) {
        d.outFile.close();
    }
    setDisconnected();
}

/********************************************************/
