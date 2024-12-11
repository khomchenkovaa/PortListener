#ifndef XMODBUSTCPSERVER_H
#define XMODBUSTCPSERVER_H

#include <QModbusTcpServer>
#include <QObject>

class XModbusTcpServer : public QModbusTcpServer
{
    Q_OBJECT

public:
    explicit XModbusTcpServer(QObject *parent = nullptr);

protected:
    QModbusResponse processPrivateRequest(const QModbusPdu &request);
    QModbusResponse processRequest(const QModbusPdu &request);
};

#endif // XMODBUSTCPSERVER_H
