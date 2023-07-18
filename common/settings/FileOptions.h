#ifndef FILEOPTIONS_H
#define FILEOPTIONS_H

#include "global.h"

#define GRP_FILEHANDLER "FileHandler"
#define APP_FILENAME    "FileName"
#define APP_FILEAPPEND  "FileAppend"

// config defaults
#define DEF_FILENAME   "/tmp/udp_log.txt"
#define DEF_FILEAPPEND true

namespace Settings {

/******************************************************************/

struct FileOptions {
    QString fileName;
    bool    fileAppend;

    void load(QSettings::Scope scope = QSettings::UserScope) {
        QSettings s(scope, ORGANIZATION_NAME, APP_CFG);
        s.beginGroup(GRP_FILEHANDLER);
        fileName   = s.value(APP_FILENAME,   DEF_FILENAME).toString();
        fileAppend = s.value(APP_FILEAPPEND, DEF_FILEAPPEND).toBool();
        s.endGroup();
    };

    void save(QSettings::Scope scope = QSettings::UserScope) {
        QSettings s(scope, ORGANIZATION_NAME, APP_CFG);
        s.beginGroup(GRP_FILEHANDLER);
        s.setValue(APP_FILENAME,   fileName);
        s.setValue(APP_FILEAPPEND, fileAppend);
        s.endGroup();
        s.sync();
    };

    static FileOptions get(QSettings::Scope scope = QSettings::UserScope) {
        FileOptions opt;
        opt.load(scope);
        return opt;
    }
};

/******************************************************************/

}

#endif // FILEOPTIONS_H
