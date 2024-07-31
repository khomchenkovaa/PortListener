#include "dbhandler.h"

#include <QSqlQuery>
#include <QSqlError>

/********************************************************/

DbHandler::DbHandler(QObject *parent)
    : MessageHandler(tr("DB handler"), parent)
{
    dbuuid = QUuid::createUuid();
}

/********************************************************/

DbHandler::~DbHandler()
{
    DbHandler::doDisconnect();
}

/********************************************************/

QByteArray DbHandler::processData(const QString &data)
{
    clearErrors();
    if (!isConnected() || !db.isOpen()) {
        addError(tr("Cannot write data to the closed database"));
        return QByteArray();
    }

    auto pair = CsvDbMapper::splitToPrefixAndCsv(data);
    const QString sql = mapper.sqlInsert(pair.first);
    QVariantMap values = mapper.bindValues(pair.first, pair.second);
    if (sql.isEmpty()) {
        addError(tr("Cannot create sql statement"));
        return QByteArray();
    }

    QSqlQuery query(db);
    query.prepare(sql);
    QMapIterator<QString, QVariant> i(values);
    while (i.hasNext()) {
        i.next();
        query.bindValue(i.key(), i.value());
    }
    if (!query.exec()) {
        addError(query.lastError().text());
    }
    return QByteArray();
}

/********************************************************/

void DbHandler::doConnect(bool binary)
{
    assert(!dbuuid.isNull());

    clearErrors();

    if (binary) {
        addError("Unable to use binary mode");
        return;
    }

    mapper.load();
    if (mapper.hasError()) {
        addError(mapper.loadError());
        return;
    }

    const QString driver   = settings()->value(Settings::DbDriver, "QPSQL").toString();
    const QString hostname = settings()->value(Settings::DbHostname, "localhost").toString();
    const int     port     = settings()->value(Settings::DbPort, 0).toInt();
    const QString username = settings()->value(Settings::DbUsername).toString();
    const QString password = settings()->value(Settings::DbPassword).toString();
    const QString database = settings()->value(Settings::DbDatabase).toString();

    if (!QSqlDatabase::isDriverAvailable(driver)) {
        addError(tr("Database driver %1 is not available.").arg(driver));
        return;
    }

    db = QSqlDatabase::addDatabase(driver, dbuuid.toString());
    db.setHostName(hostname);
    if (port > 0) db.setPort(port);
    db.setDatabaseName(database);
    db.setUserName(username);
    db.setPassword(password);

    if (!db.open()) {
        addError(db.lastError().text());
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(dbuuid.toString());
        return;
    }

    setConnected();
}

/********************************************************/

void DbHandler::doDisconnect()
{
    if (db.isOpen()) {
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(dbuuid.toString());
    }
    mapper.clear();

    setDisconnected();
}

/********************************************************/
