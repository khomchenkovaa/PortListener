#ifndef GATEHANDLER_H
#define GATEHANDLER_H

#include "messagehandler.h"
#include "gatehandlerconf.h"

#include <QFile>

class GateHandler : public MessageHandler
{
    Q_OBJECT

    struct GateHandlerPrivate {
        QFile outFile;
    };
public:
    enum Settings {
        CsvFileName,
        DefFileName,
        OutFileName,
        OutFileAppend
    };

    explicit GateHandler(QObject *parent = nullptr);

    ~GateHandler();

    QByteArray processData(const QByteArray& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    GateHandlerPrivate d;
};

#endif // GATEHANDLER_H
