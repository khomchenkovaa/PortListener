#ifndef DBHANDLER_H
#define DBHANDLER_H

#include "dbhandlerconf.h"
#include "messagehandler.h"

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

    void handleMessage(Message *msg);
    QByteArray processData(const QString& data);
    void doConnect(bool binary = false);
    void doDisconnect();
    MessageHandlerWgt *settingsWidget(QWidget *parent = nullptr) const;

private:
    QUuid		 dbuuid;
    QSqlDatabase db;
    CsvDbMapper  mapper;
};

#endif // DBHANDLER_H
