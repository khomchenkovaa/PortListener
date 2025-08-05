#ifndef DEFHANDLER_H
#define DEFHANDLER_H

#include "messagehandler.h"
#include "defhandlerconf.h"

class DEPWorker;

/// Data exchange protocol
class DefHandler : public MessageHandler
{
    Q_OBJECT

    struct DefHandlerPrivate {
        Gate::DefConfig defConf;
    };
public:
    enum Settings {
        DefFileName
    };

    explicit DefHandler(QObject *parent = nullptr);

    ~DefHandler();

    QByteArray processData(const QByteArray& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    DefHandlerPrivate d;
};

#endif // DEFHANDLER_H
