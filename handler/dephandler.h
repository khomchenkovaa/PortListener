#ifndef DEPHANDLER_H
#define DEPHANDLER_H

#include "messagehandler.h"
#include "dephandlerconf.h"

#include <QFile>

class DepHandler : public MessageHandler
{
    Q_OBJECT

    struct GateHandlerPrivate {
        Gate::CsvConf csvConf;
        Gate::DefConf defConf;
        QFile         outFile;
    };
public:
    enum Settings {
        CsvFileName,
        DefFileName,
        OutFileName,
        FileAppend
    };

    explicit DepHandler(QObject *parent = nullptr);

    ~DepHandler();

    QByteArray processData(const QByteArray& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    GateHandlerPrivate d;
};

#endif // DEPHANDLER_H
