#include "xmodbustcpserver.h"

#include <QTcpSocket>
#include <QHostAddress>

XModbusTcpConnectionObserver::XModbusTcpConnectionObserver(QObject *parent)
    : QObject(parent)
{
}

XModbusTcpConnectionObserver::~XModbusTcpConnectionObserver() {

}

bool XModbusTcpConnectionObserver::acceptNewConnection(QTcpSocket *newClient) {
    emit newConnection(QString("%1 connected to server!").arg(newClient->peerAddress().toString()));
    return true;
}

/********************************************************/

XModbusTcpServer::XModbusTcpServer(QObject *parent)
    : QModbusTcpServer{parent}
{
    XModbusTcpConnectionObserver *observer = new XModbusTcpConnectionObserver(this);
    installConnectionObserver(observer);
    connect(observer, &XModbusTcpConnectionObserver::newConnection,
            this, &XModbusTcpServer::connectInfo);
    connect(this, &QModbusTcpServer::modbusClientDisconnected, this, [this](QTcpSocket *modbusClient){
        emit connectInfo(QString("%1 disconnected from server!").arg(modbusClient->peerAddress().toString()));
    });
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

