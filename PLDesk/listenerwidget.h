#ifndef LISTENERWIDGET_H
#define LISTENERWIDGET_H

#include "messagehandler.h"

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTextBrowser)

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
        GateActionHandler
    };

    struct ListenerWidgetData {
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
    bool initHandler(bool binaryInput = true);
    void disconnectHandler();
    MessageHandler *handler() const;
    MessageHandlerWgt *updateHandler(int index);
    QByteArray doHandle(PMessage data);
    QByteArray doHandle(const QByteArray& data);
    QByteArray doHandle(const QString& data);
    QStringList handlerErrors() const;
    void clearErrors();
    virtual QTextBrowser *textLog() const {
        return Q_NULLPTR;
    }

protected Q_SLOTS:
    void printInfo(const QString& host, const QString& msg);
    void printMessage(const QString& host, const QString& msg);
    void printError(const QString& host, const QString& msg);
    void printLog(const QString& msg);

protected:
    static QStringList handlers();

protected:
    ListenerWidgetData d;
};

#endif // LISTENERWIDGET_H
