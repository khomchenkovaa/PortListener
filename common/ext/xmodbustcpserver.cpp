#include "xmodbustcpserver.h"

#include <QDebug>

/********************************************************/

XModbusTcpServer::XModbusTcpServer(QObject *parent)
    : QModbusTcpServer{parent}
{

}

/********************************************************/

QModbusResponse XModbusTcpServer::processPrivateRequest(const QModbusPdu &request)
{
    qDebug() << "processPrivateRequest" << QString::fromLatin1(request.data().toHex());
    return QModbusServer::processPrivateRequest(request);
}

/********************************************************/

QModbusResponse XModbusTcpServer::processRequest(const QModbusPdu &request)
{
    qDebug() << "processRequest" << QString::fromLatin1(request.data().toHex());
    return QModbusTcpServer::processRequest(request);
}

/********************************************************/
