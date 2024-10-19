#include "listenerwidget.h"

#include "filehandler.h"
#include "dbhandler.h"
#include "udphandler.h"
#include "tcphandler.h"
#include "sockhandler.h"
#include "gatehandler.h"
#include "modbushandler.h"

#include "filehandlerwidget.h"
#include "dbhandlerwidget.h"
#include "sockhandlerwidget.h"
#include "tcphandlerwidget.h"
#include "udphandlerwidget.h"
#include "gatehandlerwidget.h"
#include "modbushandlerwidget.h"

/********************************************************/

ListenerWidget::ListenerWidget(QWidget *parent)
    : QWidget(parent)
{

}

/********************************************************/

QString ListenerWidget::handlerName() const
{
    if (d.handler) return d.handler->name();
    return "Unknown";
}

/********************************************************/

void ListenerWidget::initHandler(bool binaryInput)
{
    if (!d.handler) return;
    if (d.editor) {
        d.handler->setSettings(d.editor->settings());
    }
    d.handler->doConnect(binaryInput);
}

/********************************************************/

void ListenerWidget::disconnectHandler()
{
    if (d.handler) {
        d.handler->doDisconnect();
    }
}

/********************************************************/

MessageHandlerWgt *ListenerWidget::updateHandler(int index)
{
    if (d.editor) {
        d.editor->deleteLater();
        d.editor = Q_NULLPTR;
    }
    if (d.handler) {
        d.handler->deleteLater();
        d.handler = Q_NULLPTR;
    }
    switch (index) {
    case ActionHandler::NoActionHandler:
        break;
    case ActionHandler::FileActionHandler:
        d.handler = new FileHandler(this);
        d.editor  = new FileHandlerWidget(this);
        break;
    case ActionHandler::DbActionHandler:
        d.handler = new DbHandler(this);
        d.editor  = new DbHandlerWidget(this);
        break;
    case ActionHandler::UdpActionHandler:
        d.handler = new UdpHandler(this);
        d.editor  = new UdpHandlerWidget(this);
        break;
    case ActionHandler::TcpActionHandler:
        d.handler = new TcpHandler(this);
        d.editor  = new TcpHandlerWidget(this);
        break;
    case ActionHandler::SocketActionHandler:
        d.handler = new SockHandler(this);
        d.editor  = new SockHandlerWidget(this);
        break;
    case ActionHandler::GateActionHandler:
        d.handler = new GateHandler(this);
        d.editor  = new GateHandlerWidget(this);
        break;
    case ActionHandler::ModbusActionHandler:
        d.handler = new ModbusHandler(this);
        d.editor  = new ModbusHandlerWidget(this);
        break;
    }
    return d.editor;
}

/********************************************************/

QByteArray ListenerWidget::doHandle(const QByteArray &data)
{
    if (d.handler) return d.handler->processData(data);
    return QByteArray();
}

/********************************************************/

QByteArray ListenerWidget::doHandle(const QString &data)
{
    if (d.handler) return d.handler->processData(data);
    return QByteArray();
}

/********************************************************/

QStringList ListenerWidget::handlerErrors() const
{
    if (d.handler) return d.handler->errors();
    return QStringList();
}

/********************************************************/

QStringList ListenerWidget::handlers()
{
    return QStringList()
            << tr("No Action")
            << tr("Save to File")
            << tr("CSV to DB")
            << tr("Send to UDP")
            << tr("Send to TCP")
            << tr("Send to Socket")
            << tr("Sacor Gate")
            << tr("Modbus L2COM");
}

/********************************************************/
