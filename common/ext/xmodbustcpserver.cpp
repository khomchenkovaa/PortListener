#include "xmodbustcpserver.h"

#include <QTcpSocket>
#include <QHostAddress>

XModbusTcpConnectionObserver::XModbusTcpConnectionObserver(XModbusTcpServer *parent)
{
    m_Server = parent;
}

XModbusTcpConnectionObserver::~XModbusTcpConnectionObserver() {
    m_Server = Q_NULLPTR;
}

bool XModbusTcpConnectionObserver::acceptNewConnection(QTcpSocket *newClient) {
    if (m_Server) {
        m_Server->modbusClientConnected(newClient->peerAddress().toString());
    }
    return true;
}

/********************************************************/

XModbusTcpServer::XModbusTcpServer(QObject *parent)
    : QModbusTcpServer{parent}
{
    XModbusTcpConnectionObserver *observer = new XModbusTcpConnectionObserver(this);
    installConnectionObserver(observer);
    connect(this, &QModbusTcpServer::modbusClientDisconnected, this, [this](QTcpSocket *modbusClient){
        emit connectInfo(QString("%1 disconnected from server!").arg(modbusClient->peerAddress().toString()));
    });
}

void XModbusTcpServer::modbusClientConnected(const QString &peerAdderss)
{
    emit connectInfo(QString("%1 connected to server!").arg(peerAdderss));
}

/********************************************************/

QModbusResponse XModbusTcpServer::processPrivateRequest(const QModbusPdu &request)
{
    emit debugInfo(QString("processPrivateRequest %1").arg(QString::fromLatin1(request.data().toHex())));
    return QModbusServer::processPrivateRequest(request);
}

/********************************************************/

QModbusResponse XModbusTcpServer::processRequest(const QModbusPdu &request)
{
    emit debugInfo(QString("processRequest %1").arg(QString::fromLatin1(request.data().toHex())));
    return QModbusTcpServer::processRequest(request);
}

/********************************************************/

