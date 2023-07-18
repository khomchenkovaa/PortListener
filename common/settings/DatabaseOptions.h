#ifndef DATABASEOPTIONS_H
#define DATABASEOPTIONS_H

#include "global.h"

#define GRP_DBHANDLER "DbHandler"
#define APP_DBHOST    "DbHostname"
#define APP_DBPORT    "DbPort"
#define APP_DBDRIVER  "DbDriver"
#define APP_DBUSER    "DbUsername"
#define APP_DBPASS    "DbPassword"
#define APP_DATABASE  "DbDatabase"

// config defaults
#define DEF_DBHOST   "localhost"
#define DEF_DBPORT   5432
#define DEF_DBDRIVER "QPSQL"
#define DEF_DBUSER   "abn_user"
#define DEF_DBPASS   "abnn"
#define DEF_DATABASE "abn_db"

namespace Settings {

/******************************************************************/

struct DatabaseOptions {
    QString host;
    int     port;
    QString driver;
    QString username;
    QString password;
    QString database;

    void load(QSettings::Scope scope = QSettings::UserScope) {
        QSettings s(scope, ORGANIZATION_NAME, APP_CFG);
        s.beginGroup(GRP_DBHANDLER);
        host     = s.value(APP_DBHOST,   DEF_DBHOST).toString();
        port     = s.value(APP_DBPORT,   DEF_DBPORT).toInt();
        driver   = s.value(APP_DBDRIVER, DEF_DBDRIVER).toString();
        username = s.value(APP_DBUSER,   DEF_DBUSER).toString();
        password = s.value(APP_DBPASS,   DEF_DBPASS).toString();
        database = s.value(APP_DATABASE, DEF_DATABASE).toString();
        s.endGroup();
    };

    void save(QSettings::Scope scope = QSettings::UserScope) {
        QSettings s(scope, ORGANIZATION_NAME, APP_CFG);
        s.beginGroup(GRP_DBHANDLER);
        s.setValue(APP_DBHOST,   host);
        s.setValue(APP_DBPORT,   port);
        s.setValue(APP_DBDRIVER, driver);
        s.setValue(APP_DBUSER,   username);
        s.setValue(APP_DBPASS,   password);
        s.setValue(APP_DATABASE, database);
        s.endGroup();
        s.sync();
    };

    static DatabaseOptions get(QSettings::Scope scope = QSettings::UserScope) {
        DatabaseOptions opt;
        opt.load(scope);
        return opt;
    }
};

/******************************************************************/

}

#endif // DATABASEOPTIONS_H
