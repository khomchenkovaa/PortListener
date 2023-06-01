#include "dbhandler.h"

#include "dbhandlerwidget.h"

#include <QSqlQuery>
#include <QSqlError>

/********************************************************/

DbHandler::DbHandler(QObject *parent)
    : MessageHandler(parent)
{
    m_Name = tr("DB handler");
    dbuuid = QUuid::createUuid();
}

/********************************************************/

DbHandler::~DbHandler()
{
    disconnect();
}

/********************************************************/

void DbHandler::handleMessage(Message *msg)
{
    processData(msg->payload.toString());
}

/********************************************************/

QByteArray DbHandler::processData(const QString &data)
{
    m_Error.clear();
    if (!m_Connected || !db.isOpen()) {
        m_Error = tr("Cannot write data to the closed database");
        return QByteArray();
    }

    auto pair = CsvDbMapper::splitToPrefixAndCsv(data);
    const QString sql = mapper.sqlInsert(pair.first);
    QVariantMap values = mapper.bindValues(pair.first, pair.second);
    if (sql.isEmpty()) {
        m_Error = tr("Cannot create sql statement");
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
        m_Error = query.lastError().text();
    }
    return QByteArray();
}

/********************************************************/

void DbHandler::connect(bool binary)
{
    assert(!dbuuid.isNull());

    m_Error.clear();

    if (binary) {
        m_Error = "Unable to use binary mode";
        return;
    }

    mapper.load();
    if (mapper.hasError()) {
        m_Error = mapper.loadError();
        return;
    }

    const QString driver   = m_Settings.value(Settings::DbDriver, "QPSQL").toString();
    const QString hostname = m_Settings.value(Settings::DbHostname, "localhost").toString();
    const int     port     = m_Settings.value(Settings::DbPort, 0).toInt();
    const QString username = m_Settings.value(Settings::DbUsername).toString();
    const QString password = m_Settings.value(Settings::DbPassword).toString();
    const QString database = m_Settings.value(Settings::DbDatabase).toString();

    if (!QSqlDatabase::isDriverAvailable(driver)) {
        m_Error = QString("Database driver %1 is not available.").arg(driver);
        return;
    }

    db = QSqlDatabase::addDatabase(driver, dbuuid.toString());
    db.setHostName(hostname);
    if (port > 0) db.setPort(port);
    db.setDatabaseName(database);
    db.setUserName(username);
    db.setPassword(password);

    if (!db.open()) {
        m_Error = db.lastError().text();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(dbuuid.toString());
        return;
    }

    m_Connected = true;
}

/********************************************************/

void DbHandler::disconnect()
{
    if (db.isOpen()) {
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(dbuuid.toString());
    }
    mapper.clear();

    m_Connected = false;
}

/********************************************************/

MessageHandlerWgt *DbHandler::settingsWidget(QWidget *parent) const
{
    return new DbHandlerWidget(parent);
}

/********************************************************/
