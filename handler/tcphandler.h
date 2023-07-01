#ifndef TCPHANDLER_H
#define TCPHANDLER_H

#include "messagehandler.h"

#include <QTcpSocket>

class TcpHandler : public MessageHandler
{
     Q_OBJECT

public:
    enum Settings {
        Host,
        Port
    };

    explicit TcpHandler(QObject *parent = nullptr);

    ~TcpHandler();

    void handleMessage(Message *msg);
    QByteArray processData(const QByteArray& data);
    QByteArray processData(const QString& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    QTcpSocket *m_TcpSocket;

};

#endif // TCPHANDLER_H
