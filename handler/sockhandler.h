#ifndef SOCKHANDLER_H
#define SOCKHANDLER_H

#include "messagehandler.h"

#include <QLocalSocket>

class SockHandler : public MessageHandler
{
     Q_OBJECT

public:
    enum Settings {
        Socket
    };

    explicit SockHandler(QObject *parent = nullptr);

    ~SockHandler();

    void handleMessage(Message *msg);
    QByteArray processData(const QByteArray& data);
    QByteArray processData(const QString& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    QLocalSocket *m_LocalSocket;

};

#endif // SOCKHANDLER_H
