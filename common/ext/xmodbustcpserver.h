#ifndef XMODBUSTCPSERVER_H
#define XMODBUSTCPSERVER_H

#include <QModbusTcpServer>

class XModbusTcpServer;

class XModbusTcpConnectionObserver : public QModbusTcpConnectionObserver
{
public:
    XModbusTcpConnectionObserver(XModbusTcpServer *parent = Q_NULLPTR);
    ~XModbusTcpConnectionObserver();
    bool acceptNewConnection(QTcpSocket *newClient);

private:
    XModbusTcpServer *m_Server = Q_NULLPTR;
};

class XModbusTcpServer : public QModbusTcpServer
{
    Q_OBJECT

public:
    explicit XModbusTcpServer(QObject *parent = nullptr);

    void modbusClientConnected(const QString &peerAdderss);

Q_SIGNALS:
    void connectInfo(const QString& msg);
    void debugInfo(const QString& msg);

protected:
    QModbusResponse processPrivateRequest(const QModbusPdu &request);
    QModbusResponse processRequest(const QModbusPdu &request);
};

#endif // XMODBUSTCPSERVER_H
