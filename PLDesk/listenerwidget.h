#ifndef LISTENERWIDGET_H
#define LISTENERWIDGET_H

#include "messagehandler.h"

#include <QWidget>

class ListenerWidget : public QWidget
{
    Q_OBJECT

    enum ActionHandler {
        NoActionHandler,
        FileActionHandler,
        DbActionHandler,
        UdpActionHandler,
        TcpActionHandler,
        SocketActionHandler,
        GateActionHandler,
        ModbusActionHandler
    };

    struct ListenerWidgetPrivate {
        MessageHandler    *handler = Q_NULLPTR;
        MessageHandlerWgt *editor  = Q_NULLPTR;
    };

public:
    enum ReplyType {
        NoReply,
        EchoReply,
        TextReply,
        BinaryReply,
        ActionReply
    };

    explicit ListenerWidget(QWidget *parent = nullptr);

protected:
    QString handlerName() const;
    void initHandler(bool binaryInput);
    void disconnectHandler();
    MessageHandlerWgt *updateHandler(int index);
    QByteArray doHandle(const QByteArray& data);
    QByteArray doHandle(const QString& data);
    QStringList handlerErrors() const;

protected:
    static QStringList handlers();

private:
    ListenerWidgetPrivate d;
};

#endif // LISTENERWIDGET_H
