#ifndef LISTENERWIDGET_H
#define LISTENERWIDGET_H

#include "messagehandler.h"

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTextBrowser)

class ListenerWidget : public QWidget
{
    Q_OBJECT

    enum DecodeHandler {
        NoDecodeHandler,
        DbDecodeHandler,
        DepDecodeHandler
    };

    enum ActionHandler {
        NoActionHandler,
        FileActionHandler,
        UdpActionHandler,
        TcpActionHandler,
        SocketActionHandler,
        MqueueActionHandler
    };

    struct ListenerWidgetData {
        MessageHandler    *decodeHandler = Q_NULLPTR;
        MessageHandlerWgt *decodeEditor  = Q_NULLPTR;
        MessageHandler    *actionHandler = Q_NULLPTR;
        MessageHandlerWgt *actionEditor  = Q_NULLPTR;
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
    // decoders
    QString decoderName() const;
    bool isDecoderConnected() const;
    bool initDecoder(bool binaryInput = true);
    void disconnectDecoder();
    MessageHandler *decoder() const;
    MessageHandlerWgt *updateDecoder(int index);
    QByteArray doDecode(PMessage data);
    QByteArray doDecode(const QByteArray& data);
    QByteArray doDecode(const QString& data);
    QStringList decoderErrors() const;
    // action handlers
    QString handlerName() const;
    bool isHandlerConnected() const;
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
    void printReply(const QString& host, const QString& msg);
    void printError(const QString& host, const QString& msg);
    void printLog(const QString& msg);

protected:
    static QStringList decoders();
    static QStringList handlers();

protected:
    ListenerWidgetData d;
};

#endif // LISTENERWIDGET_H
