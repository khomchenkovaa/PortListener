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
    while (!d.depWorker->isBufferEmpty()) {
        if (!d.depWorker->doWork()) break;
        const auto records = d.depWorker->packetData().records;
        for (const auto &rec : records) {
            emit logMessage(name(), rec.toString());
        }
    }
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

    return data;
}

/********************************************************/

void DepHandler::doConnect(bool binary)
{
    clearErrors();
    if (!binary) {
        addError(tr("Only binary mode alowed"));
        return;
    }
    const auto csvFileName = settings()->value(Settings::CsvFileName).toString();
    if (csvFileName.isEmpty()) {
        emit logMessage(name(), tr("No CSV config file to open"));
    } else {
        d.csvConf.csv.typeValue   = settings()->value(Settings::TypeValue, "NONE").toString();
        d.csvConf.csv.typeColumn  = settings()->value(Settings::TypeColumn, -1).toInt();
        d.csvConf.csv.indexColumn = settings()->value(Settings::IndexColumn, -1).toInt();
        d.csvConf.csv.kksColumn   = settings()->value(Settings::KksColumn, -1).toInt();
        d.csvConf.csv.iidColumn   = settings()->value(Settings::IidColumn, -1).toInt();
        int resId = d.csvConf.load(csvFileName, ',', QTextCodec::codecForName("Windows-1251"));
        switch (resId) {
        case -1:
            addError(tr("Load CSV config file failed! Not enought column to load values"));
            break;
        case -2:
            addError(tr("Load CSV config file failed! Type value is not defined"));
            break;
        case -3:
            addError(tr("Load CSV config file failed! Index is not defined"));
            break;
        case -4:
            addError(tr("Load CSV config file failed! KKS or IID is not defined"));
            break;
        default:
            emit logMessage(name(), tr("Loaded %1 items from CSV config file").arg(resId));
            break;
        }
    }

    d.depWorker = new DEPWorker(this);
    connect(d.depWorker, &DEPWorker::signalError, this, [this](const QString& msg){
        emit logError(name(), msg);
    });
    connect(d.depWorker, &DEPWorker::signalMsg, this, [this](const QString& msg){
        emit logMessage(name(), msg);
    });
//    connect(d.depWorker, &DEPWorker::dataReceived, this, [this](const DEPData& data){
//        for (const auto &rec : data.records) {
//            emit logMessage(name(), rec.toString());
//        }
//    });

    setConnected();
}

/********************************************************/

void DepHandler::doDisconnect()
{
    setDisconnected();
}

/********************************************************/
