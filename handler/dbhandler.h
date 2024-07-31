#ifndef DBHANDLER_H
#define DBHANDLER_H

#include "messagehandler.h"
#include "dbhandlerconf.h"

#include <QSqlDatabase>

class DbHandler : public MessageHandler
{
    Q_OBJECT

public:
    enum Settings {
        DbHostname,
        DbPort,
        DbDriver,
        DbUsername,
        DbPassword,
        DbDatabase
    };

    explicit DbHandler(QObject *parent = nullptr);
    ~DbHandler();

    QByteArray processData(const QString& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    QUuid		 dbuuid;
    QSqlDatabase db;
    CsvDbMapper  mapper;
};

#endif // DBHANDLER_H
