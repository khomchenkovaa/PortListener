#ifndef MODBUSOPTIONS_H
#define MODBUSOPTIONS_H

#include "global.h"

#include <QCoreApplication>

#define MB_CONFIG_FILE "scrload.conf"
//INPUT_METHOD=NETWORK
//HOST1=gate1
//HOST2=gate2
//PORT=1024
//FREQUENCY=4
//UNIT=tnv1
//OUTPUT_DIR=/export/home/ska/sacor/tmp
//LOG_SIZE=100000
//LOG_LEVEL=VERY_LOW

#define MB_SIG_FILE "scrload.sig"

// config keys
#define MB_INPUT_METHOD "INPUT_METHOD"
#define MB_HOST1        "HOST1"
#define MB_HOST2        "HOST2"
#define MB_PORT         "PORT"
#define MB_FREQUENCY    "FREQUENCY"
#define MB_UNIT         "UNIT"
#define MB_OUTPUT_DIR   "OUTPUT_DIR"
#define MB_LOG_SIZE     "LOG_SIZE"
#define MB_LOG_LEVEL    "LOG_LEVEL"

// config defaults
#define DEF_INPUT_METHOD "NETWORK"
#define DEF_HOST1        "gate1"
#define DEF_HOST2        "gate2"
#define DEF_UNIT         "tnv1"
#define DEF_OUTPUT_DIR   "/export/home/ska/sacor/tmp"
#define DEF_LOG_LEVEL    "VERY_LOW"

// modbus
#define GRP_MODBUS "Modbus"
#define MB_TIMEOUT "Timeout"
#define MB_RETRIES "Retries"
#define MB_SRV_ID  "ServerId"

enum {
    DEF_PORT      = 1024,
    DEF_FREQUENCY = 4,
    DEF_LOG_SIZE  = 100000
};

namespace Settings {

/// SACOR input data preparation module configuration file
struct ModbusOptions {
    QString inputMethod;
    QString host1;
    QString host2;
    int     port = -1;
    int     frequency = -1;
    QString unit;
    QString outputDir;
    int     logSize = -1;
    QString logLevel;
    QString optFilePath;

    int timeout  = 1;
    int retries  = 3;
    int serverId = 1;

    void load() {
        QSettings s(QSettings::NativeFormat,
                    QSettings::SystemScope,
                    QCoreApplication::organizationName(),
                    QCoreApplication::applicationName());
        inputMethod = s.value(MB_INPUT_METHOD, DEF_INPUT_METHOD).toString();
        host1       = s.value(MB_HOST1, DEF_HOST1).toString();
        host2       = s.value(MB_HOST2, DEF_HOST2).toString();
        port        = s.value(MB_PORT, DEF_PORT).toInt();
        frequency   = s.value(MB_FREQUENCY, DEF_FREQUENCY).toInt();
        unit        = s.value(MB_UNIT, DEF_UNIT).toString();
        outputDir   = s.value(MB_OUTPUT_DIR, DEF_OUTPUT_DIR).toString();
        logSize     = s.value(MB_LOG_SIZE, DEF_LOG_SIZE).toInt();
        logLevel    = s.value(MB_LOG_LEVEL, DEF_LOG_LEVEL).toString();
        optFilePath = s.fileName();

        s.beginGroup(GRP_MODBUS);
        timeout  = s.value(MB_TIMEOUT, timeout).toInt();
        retries  = s.value(MB_RETRIES, retries).toInt();
        serverId = s.value(MB_SRV_ID, serverId).toInt();
        s.endGroup();
    };

    void save() {
        QSettings s(QSettings::NativeFormat,
                    QSettings::SystemScope,
                    QCoreApplication::organizationName(),
                    QCoreApplication::applicationName());
        s.setValue(MB_INPUT_METHOD, inputMethod);
        s.setValue(MB_HOST1, host1);
        s.setValue(MB_HOST2, host2);
        s.setValue(MB_PORT, port);
        s.setValue(MB_FREQUENCY, frequency);
        s.setValue(MB_UNIT, unit);
        s.setValue(MB_OUTPUT_DIR, outputDir);
        s.setValue(MB_LOG_SIZE, logSize);
        s.setValue(MB_LOG_LEVEL, logLevel);
        s.sync();
    };

    static ModbusOptions get() {
        ModbusOptions opt;
        opt.load();
        return opt;
    }
};

}
#endif // MODBUSOPTIONS_H
