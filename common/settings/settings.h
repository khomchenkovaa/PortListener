#ifndef SETTINGS_H
#define SETTINGS_H

#define CONFIG(TYPE) QCoreApplication::instance()->findChild< TYPE *>()

#include <QSettings>

// common
#define ORGANIZATION_NAME   "SNIIP"
#define ORGANIZATION_DOMAIN "sniip.ru"
#define APPLICATION_NAME    "PortListener"
#define APPLICATION_STYLE   "Windows"
#define APP_POINT_SIZE      8

#define APP_NAMESPACE       SNIIP

// date-time format
#define DATE_TIME_FORMAT "yyyy-MM-dd HH:mm"
#define DATE_FORMAT      "yyyy-MM-dd"

// config
#define APP_CFG "config"

#endif // SETTINGS_H
