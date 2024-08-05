#ifndef LISTENERWIDGET_H
#define LISTENERWIDGET_H

#include "messagehandler.h"
#include "iodecoder.h"

#include <QWidget>

class ListenerWidget : public QWidget
{
    Q_OBJECT

public:
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
        SocketActionHandler,
        GateActionHandler
    };

    explicit ListenerWidget(QWidget *parent = nullptr);

protected:
    MessageHandlerWgt *updateHandler(int index);

protected:
    IODecoder       ioDecoder;
    MessageHandler *m_Handler;
};

#endif // LISTENERWIDGET_H
