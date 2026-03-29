#ifndef XMODBUSTCPSERVER_H
#define XMODBUSTCPSERVER_H

#include <QModbusTcpServer>
#include <QObject>

class XModbusTcpConnectionObserver : public QObject, public QModbusTcpConnectionObserver {
    Q_OBJECT

public:
    XModbusTcpConnectionObserver(QObject *parent = Q_NULLPTR);
    ~XModbusTcpConnectionObserver();
    bool acceptNewConnection(QTcpSocket *newClient);

Q_SIGNALS:
    void newConnection(const QString& clientInfo);
};

class XModbusTcpServer : public QModbusTcpServer
{
    Q_OBJECT

public:
    explicit XModbusTcpServer(QObject *parent = nullptr);

Q_SIGNALS:
    void connectInfo(const QString& msg);
    void debugInfo(const QString& msg);

protected:
    QModbusResponse processPrivateRequest(const QModbusPdu &request);
    QModbusResponse processRequest(const QModbusPdu &request);
};

#endif // XMODBUSTCPSERVER_H
