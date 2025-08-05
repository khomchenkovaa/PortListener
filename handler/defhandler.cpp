#include "defhandler.h"

#include <QTextStream>
#include <QDataStream>

/********************************************************/

DefHandler::DefHandler(QObject *parent)
    : MessageHandler(tr("Def handler"), parent)
{

}

/********************************************************/

DefHandler::~DefHandler()
{
    DefHandler::doDisconnect();
}

/********************************************************/

QByteArray DefHandler::processData(const QByteArray &data)
{
    clearErrors();
    for (int i=0; i < d.defConf.itemsCount(); ++i) {
        QString name = d.defConf.name(i);
        Gate::DefDataType typeId = d.defConf.typeId(i);
        const QByteArray ba = data.mid(d.defConf.offset(i), d.defConf.size(i));
        QDataStream ds(ba);

        switch(typeId) {
        case Gate::defTimeval:
        { // type_timeval
            int time = 0;
            ds >> time;
            emit logMessage(name, QString::number(time));
        } break;
        case Gate::defLongInt:
        { // type_longint
            qint32 value = 0;
            ds >> value;
            emit logMessage(name, QString::number(value));
        } break;
        case Gate::defFloat:
        { // type_float
            float value = 0.0;
            ds >> value;
            emit logMessage(name, QString::number(value, 'f', 2));
        } break;
        case Gate::defDouble:
        { // type_double
            qreal value = 0.0;
            ds >> value;
            emit logMessage(name, QString::number(value, 'f', 2));
        } break;
        case Gate::defBinByte:
        { // type_binbyte
            qint8 value = 0;
            ds >> value;
            emit logMessage(name, QString::number(value));
        } break;
        default:
            break;
        }
    }

    return data;
}

/********************************************************/

void DefHandler::doConnect(bool binary)
{
    clearErrors();
    if (!binary) {
        addError(tr("Only binary mode alowed"));
        return;
    }
    const auto defFileName = settings()->value(Settings::DefFileName).toString();
    if (defFileName.isEmpty()) {
        emit logMessage(name(), tr("No DEF config file to open"));
    } else {
        d.defConf.load(defFileName);
    }

    setConnected();
}

/********************************************************/

void DefHandler::doDisconnect()
{
    d.defConf.clear();
    setDisconnected();
}

/********************************************************/
