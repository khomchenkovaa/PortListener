#include "listenerwidget.h"

#include "filehandler.h"
#include "dbhandler.h"
#include "udphandler.h"
#include "tcphandler.h"
#include "sockhandler.h"
#include "gatehandler.h"

#include "filehandlerwidget.h"
#include "dbhandlerwidget.h"
#include "sockhandlerwidget.h"
#include "tcphandlerwidget.h"
#include "udphandlerwidget.h"
#include "gatehandlerwidget.h"

ListenerWidget::ListenerWidget(QWidget *parent)
    : QWidget(parent)
    , m_Handler(Q_NULLPTR)
{

}

/********************************************************/

MessageHandlerWgt *ListenerWidget::updateHandler(int index)
{
    MessageHandlerWgt *editor = Q_NULLPTR;
    if (m_Handler) {
        m_Handler->deleteLater();
        m_Handler = Q_NULLPTR;
    }
    switch (index) {
    case ActionHandler::NoActionHandler:
        break;
    case ActionHandler::FileActionHandler:
        m_Handler = new FileHandler(this);
        editor = new FileHandlerWidget(this);
        break;
    case ActionHandler::DbActionHandler:
        m_Handler = new DbHandler(this);
        editor = new DbHandlerWidget(this);
        break;
    case ActionHandler::UdpActionHandler:
        m_Handler = new UdpHandler(this);
        editor = new UdpHandlerWidget(this);
        break;
    case ActionHandler::TcpActionHandler:
        m_Handler = new TcpHandler(this);
        editor = new TcpHandlerWidget(this);
        break;
    case ActionHandler::SocketActionHandler:
        m_Handler = new SockHandler(this);
        editor = new SockHandlerWidget(this);
        break;
    case ActionHandler::GateActionHandler:
        m_Handler = new GateHandler(this);
        editor = new GateHandlerWidget(this);
        break;
    }
    return editor;
}
