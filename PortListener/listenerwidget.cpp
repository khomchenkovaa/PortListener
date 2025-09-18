#include "listenerwidget.h"

#include "filehandler.h"
#include "dbhandler.h"
#include "udphandler.h"
#include "tcphandler.h"
#include "sockhandler.h"
#include "dephandler.h"
#include "defhandler.h"
#include "modbushandler.h"

#include "filehandlerwidget.h"
#include "dbhandlerwidget.h"
#include "sockhandlerwidget.h"
#include "tcphandlerwidget.h"
#include "udphandlerwidget.h"
#include "dephandlerwidget.h"
#include "defhandlerwidget.h"
#include "modbushandlerwidget.h"

#ifdef MQUEUE
#include "mqueuehandler.h"
#include "mqueuehandlerwidget.h"
#endif // MQUEUE

#include <QTextBrowser>

ListenerWidget::ListenerWidget(QWidget *parent)
    : QWidget(parent)
{

}

QString ListenerWidget::decoderName() const
{
    return d.decodeHandler ? d.decodeHandler->name() : "Unknown";
}

bool ListenerWidget::isDecoderConnected() const
{
    return d.actionHandler ? d.actionHandler->isConnected() : false;
}

bool ListenerWidget::initDecoder(bool binaryInput)
{
    if (!d.decodeHandler) return false;
    if (d.decodeEditor) {
        d.decodeHandler->setSettings(d.decodeEditor->settings());
    }
    d.decodeHandler->doConnect(binaryInput);
    return true;
}

void ListenerWidget::disconnectDecoder()
{
    if (d.decodeHandler) d.decodeHandler->doDisconnect();
}

MessageHandler *ListenerWidget::decoder() const
{
    return d.decodeHandler;
}

MessageHandlerWgt *ListenerWidget::updateDecoder(int index)
{
    if (isDecoderConnected()) {
        disconnectDecoder();
    }
    if (d.decodeEditor) {
        d.decodeEditor->deleteLater();
        d.decodeEditor = Q_NULLPTR;
    }
    if (d.decodeHandler) {
        d.decodeHandler->deleteLater();
        d.decodeHandler = Q_NULLPTR;
    }
    switch (index) {
    case DecodeHandler::DbDecodeHandler:
        d.decodeHandler = new DbHandler(this);
        d.decodeEditor  = new DbHandlerWidget(this);
        break;
    case DecodeHandler::DepDecodeHandler:
        d.decodeHandler = new DepHandler(this);
        d.decodeEditor  = new DepHandlerWidget(this);
        break;
    case DecodeHandler::DefDecodeHandler:
        d.decodeHandler = new DefHandler(this);
        d.decodeEditor  = new DefHandlerWidget(this);
        break;
    default:
        break;
    }
    return d.decodeEditor;
}

QByteArray ListenerWidget::doDecode(PMessage data)
{
    return d.decodeHandler ? d.decodeHandler->handleMessage(data) : data->payload.toByteArray();
}

QByteArray ListenerWidget::doDecode(const QByteArray &data)
{
    return d.decodeHandler ? d.decodeHandler->processData(data) : data;
}

QByteArray ListenerWidget::doDecode(const QString &data)
{
    return d.decodeHandler ? d.decodeHandler->processData(data) : data.toUtf8();
}

QStringList ListenerWidget::decoderErrors() const
{
    return d.decodeHandler ? d.decodeHandler->errors() : QStringList();
}

QString ListenerWidget::handlerName() const
{
    return d.actionHandler ? d.actionHandler->name() : "Unknown";
}

bool ListenerWidget::isHandlerConnected() const
{
    return d.actionHandler ? d.actionHandler->isConnected() : false;
}

bool ListenerWidget::initHandler(bool binaryInput)
{
    if (!d.actionHandler) return false;
    if (d.actionEditor) {
        d.actionHandler->setSettings(d.actionEditor->settings());
    }
    d.actionHandler->doConnect(binaryInput);
    return true;
}

void ListenerWidget::disconnectHandler()
{
    if (d.actionHandler) d.actionHandler->doDisconnect();
}

MessageHandler *ListenerWidget::handler() const
{
    return d.actionHandler;
}

MessageHandlerWgt *ListenerWidget::updateHandler(int index)
{
    if (isHandlerConnected()) {
        disconnectHandler();
    }
    if (d.actionEditor) {
        d.actionEditor->deleteLater();
        d.actionEditor = Q_NULLPTR;
    }
    if (d.actionHandler) {
        d.actionHandler->deleteLater();
        d.actionHandler = Q_NULLPTR;
    }
    switch (index) {
    case ActionHandler::NoActionHandler:
        break;
    case ActionHandler::FileActionHandler:
        d.actionHandler = new FileHandler(this);
        d.actionEditor  = new FileHandlerWidget(this);
        break;
    case ActionHandler::UdpActionHandler:
        d.actionHandler = new UdpHandler(this);
        d.actionEditor  = new UdpHandlerWidget(this);
        break;
    case ActionHandler::TcpActionHandler:
        d.actionHandler = new TcpHandler(this);
        d.actionEditor  = new TcpHandlerWidget(this);
        break;
    case ActionHandler::SocketActionHandler:
        d.actionHandler = new SockHandler(this);
        d.actionEditor  = new SockHandlerWidget(this);
        break;
#ifdef MQUEUE
    case ActionHandler::MqueueActionHandler:
        d.actionHandler = new MqueueHandler(this);
        d.actionEditor  = new MqueueHandlerWidget(this);
        break;
#endif
    default:
        break;
    }
    return d.actionEditor;
}

QByteArray ListenerWidget::doHandle(PMessage data)
{
    return d.actionHandler ? d.actionHandler->handleMessage(data) : QByteArray();
}

QByteArray ListenerWidget::doHandle(const QByteArray &data)
{
    return d.actionHandler ? d.actionHandler->processData(data) : QByteArray();
}

QByteArray ListenerWidget::doHandle(const QString &data)
{
    return d.actionHandler ? d.actionHandler->processData(data) : QByteArray();
}

QStringList ListenerWidget::handlerErrors() const
{
    return d.actionHandler ? d.actionHandler->errors() : QStringList();
}

void ListenerWidget::clearErrors()
{
    if (d.decodeHandler) d.decodeHandler->clearErrors();
    if (d.actionHandler) d.actionHandler->clearErrors();
}

void ListenerWidget::printInfo(const QString &host, const QString &msg)
{ 
    printLog(QString("<font color=\"black\">[%1] %2 -&gt; %3</font>")
                      .arg(QTime::currentTime().toString("hh:mm:ss"), host, msg));
}

void ListenerWidget::printMessage(const QString &host, const QString &msg)
{
    printLog(QString("<font color=\"black\">[%1] %2 -&gt; </font><font color=\"darkgreen\">%3</font>")
             .arg(QTime::currentTime().toString("hh:mm:ss"), host, msg));
}

void ListenerWidget::printReply(const QString &host, const QString &msg)
{
    printLog(QString("<font color=\"black\">[%1] %2 &lt;- </font><font color=\"darkblue\">%3</font>")
             .arg(QTime::currentTime().toString("hh:mm:ss"), host, msg));
}

void ListenerWidget::printError(const QString &host, const QString &msg)
{
    printLog(QString("<font color=\"black\">[%1] %2 -&gt; </font><font color=\"red\">%3</font>")
                      .arg(QTime::currentTime().toString("hh:mm:ss"), host, msg));
}

void ListenerWidget::printLog(const QString &msg)
{
    if (!textLog()) return;
    textLog()->append(msg);
}

QStringList ListenerWidget::decoders()
{
    return QStringList()
            << tr("No Decode")
            << tr("CSV to DB")
            << tr("DEP Gate")
            << tr("DEF Stream");
}

QStringList ListenerWidget::handlers()
{
    return QStringList()
            << tr("No Action")
            << tr("Save to File")
            << tr("Send to UDP")
            << tr("Send to TCP")
            << tr("Send to Socket")
            << tr("Send to MQueue");
}
