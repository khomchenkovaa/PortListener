#include "gatehandler.h"

#include <QTextStream>

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
    return MessageHandler::processData(data);
}

/********************************************************/

void GateHandler::doConnect(bool binary)
{
    MessageHandler::doConnect(binary);
}

/********************************************************/

void GateHandler::doDisconnect()
{
    MessageHandler::doDisconnect();
}

/********************************************************/
