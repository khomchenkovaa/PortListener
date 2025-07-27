#ifndef MQUEUEHANDLER_H
#define MQUEUEHANDLER_H

#include "messagehandler.h"
#include "qmqueue.h"

class MqueueHandler : public MessageHandler
{
    Q_OBJECT

public:
    enum Settings {
        Mqueue
    };

    explicit MqueueHandler(QObject *parent = nullptr);

    ~MqueueHandler();

    QByteArray processData(const QByteArray& data);
    QByteArray processData(const QString& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    QMQueue *mq_worker;
};

#endif // MQUEUEHANDLER_H
