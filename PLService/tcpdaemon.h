#ifndef TCP_DAEMON_H
#define TCP_DAEMON_H

#include "messagehandler.h"
#include "qtservice.h"

#include "QTcpServer"
#include "QTcpSocket"
#include "QFile"
#include "QTextStream"
#include <QMetaType>

class TcpDaemon: public QTcpServer
{
	Q_OBJECT
public:
    TcpDaemon(quint16 port, QObject* parent = 0);
    ~TcpDaemon();

public slots:
    void startServer(quint16 port = 2424);
    void stopServer();
    void pauseServer();
    void resumeServer();

// QTcpServer interface
protected:
    void incomingConnection(qintptr handle) override;

private slots:
    void socketRead();
    void socketStateChanged(QAbstractSocket::SocketState socketState);
    void discardClient(QTcpSocket* socket = Q_NULLPTR);

private:
    void initHandler();
    QByteArray processData(const QByteArray &data);

private:
    bool disabled;
    MessageHandler *m_Handler;

};

#endif // TCP_DAEMON_H
