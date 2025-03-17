#include "dephandler.h"

#include "depworker.h"

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
    d.depWorker->addToBuffer(data);
//    QByteArray buff(data);
//    if (d.defConf.size() > data.size()) {
//        buff.append(d.defConf.size() - data.size(), '\0');
//    }

    QString output;
    QTextStream out(&output);

//    for (int i=0; i<d.defConf.fields(); ++i) {
//        switch(d.defConf.typeId(i)) {
//        case Gate::TIMEVAL: { // type_timeval
//            const QByteArray ba = buff.mid(d.defConf.offset(i), 8);
//            QDataStream ds(ba);
//            int time = 0;
//            ds >> time;
//            out << time << Qt::endl;
//        } break;
//        case Gate::FLOATVALID: { // type_rval
//            const QByteArray ba = buff.mid(d.defConf.offset(i), 8);
//            QDataStream ds(ba);
//            qreal value = 0.0;
//            ds >> value;
//            out << d.defConf.name(i) << " " << value << Qt::endl;
//        } break;
//        case Gate::SDWORDVALID: { // type_ival
//            const QByteArray ba = buff.mid(d.defConf.offset(i), 8);
//            QDataStream ds(ba);
//            qint64 value = 0;
//            ds >> value;
//            out << d.defConf.name(i) << " " << value << Qt::endl;
//        } break;
//        default:
//            break;
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

void DepHandler::doConnect(bool binary)
{
    clearErrors();
    const auto csvFileName = settings()->value(Settings::CsvFileName).toString();
    if (csvFileName.isEmpty()) {
        emit logMessage(name(), tr("No CSV config file to open"));
    } else {
        d.csvConf.load(csvFileName, ',', QTextCodec::codecForName("Windows-1251"));
    }
//    const auto defFileName = settings()->value(Settings::DefFileName).toString();
//    if (defFileName.isEmpty()) {
//        emit logMessage(name(), tr("No DEF config file to open"));
//    } else {
//        d.defConf.load(defFileName);
//    }

    const auto outFileName = settings()->value(Settings::OutFileName).toString();
    if (outFileName.isEmpty()) {
        emit logMessage(name(), tr("No output file to open"));
    } else {
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
    }

    d.depWorker = new DEPWorker(this);
    connect(d.depWorker, &DEPWorker::signalError, this, [this](const QString& msg){
        emit logError(name(), msg);
    });
    connect(d.depWorker, &DEPWorker::signalMsg, this, [this](const QString& msg){
        emit logMessage(name(), msg);
    });

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
