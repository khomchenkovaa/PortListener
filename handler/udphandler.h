#ifndef UDPHANDLER_H
#define UDPHANDLER_H

#include "messagehandler.h"

#include <QUdpSocket>

class UdpHandler : public MessageHandler
{
     Q_OBJECT

public:
    enum Settings {
        Host,
        Port
    };

    explicit UdpHandler(QObject *parent = nullptr);

    ~UdpHandler();

    void handleMessage(Message *msg);
    QByteArray processData(const QByteArray& data);
    QByteArray processData(const QString& data);
    void doConnect(bool binary = false);
    void doDisconnect();
    MessageHandlerWgt *settingsWidget(QWidget *parent = nullptr) const;

private:
    QUdpSocket *m_UdpSocket;

};

#endif // UDPHANDLER_H
