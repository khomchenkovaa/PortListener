#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

#include "messagehandler.h"
#include "iodecoder.h"

#include <QWidget>
#include <QLocalServer>
#include <QLocalSocket>

namespace Ui {
class SocketListener;
}

class SocketListener : public QWidget
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
    explicit SocketListener(QWidget *parent = nullptr);
    ~SocketListener();

signals:
    void tabText(const QString &label);

public slots:
    void onNewConnection();
    void onLocalSocketStateChanged(QLocalSocket::LocalSocketState socketState);
    void onReadyRead();

private slots:
    void doConnect();
    void doDisconnect();
    void onInputFormatChanged();
    void changeReplyType(int index);
    void changeHandler(int index);

private:
    void updateStatus();
    void updateCodecs();
    QByteArray processData(quintptr socketDescriptor, const QByteArray &data);

private:
    Ui::SocketListener *ui;
    QLocalServer m_LocalServer;
    IODecoder  ioDecoder;
    MessageHandler  *m_Handler;
};

#endif // SOCKETLISTENER_H
