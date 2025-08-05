#ifndef DEPHANDLER_H
#define DEPHANDLER_H

#include "messagehandler.h"
#include "dephandlerconf.h"

class DEPWorker;

/// Data exchange protocol
class DepHandler : public MessageHandler
{
    Q_OBJECT

    struct DepHandlerPrivate {
        Gate::CsvConfig csvConf;
        DEPWorker *depWorker = Q_NULLPTR; ///< обьект для обработки (паковки/распаковки DEP-пакетов)
    };
public:
    enum Settings {
        CsvFileName,
        TypeValue,
        TypeColumn,
        IndexColumn,
        KksColumn,
        IidColumn
    };

    explicit DepHandler(QObject *parent = nullptr);

    ~DepHandler();

    QByteArray processData(const QByteArray& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    DepHandlerPrivate d;
};

#endif // DEPHANDLER_H
