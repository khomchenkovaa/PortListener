#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include "messagehandler.h"
#include "iodecoder.h"

#include <QWidget>
#include <QUdpSocket>

namespace Ui {
class UdpListener;
}

class UdpListener : public QWidget
{
    Q_OBJECT

    enum ReplyType {
        NoReply,
        EchoReply,
        TextReply,
        BinaryReply,
        ActionReply
    };

    enum ActionHandler {
        NoActionHandler,
        FileActionHandler,
        DbActionHandler,
        UdpActionHandler,
        TcpActionHandler,
        SocketActionHandler
    };

public:
    explicit UdpListener(QWidget *parent = nullptr);
    ~UdpListener();

signals:
    void tabText(const QString &label);

public slots:
    void readPendingDatagrams();

private slots:
    void doConnect();
    void doDisconnect();
    void onInputFormatChanged();
    void changeReplyType(int index);
    void changeHandler(int index);

private:
    void updateStatus();
    void updateCodecs();
    QByteArray processData(const QHostAddress &host, const QByteArray &data);

private:
    Ui::UdpListener *ui;
    QUdpSocket      *m_UdpSocket;
    IODecoder        ioDecoder;
    MessageHandler  *m_Handler;
};

#endif // UDPLISTENER_H
