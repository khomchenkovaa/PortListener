INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dbhandlerwidget.h \
    $$PWD/filehandlerwidget.h \
    $$PWD/gatehandlerwidget.h \
    $$PWD/messagehandlerwgt.h \
    $$PWD/modbushandlerwidget.h \
    $$PWD/sockhandlerwidget.h \
    $$PWD/tcphandlerwidget.h \
    $$PWD/udphandlerwidget.h

FORMS += \
    $$PWD/dbhandlerwidget.ui \
    $$PWD/filehandlerwidget.ui \
    $$PWD/gatehandlerwidget.ui \
    $$PWD/modbushandlerwidget.ui \
    $$PWD/sockhandlerwidget.ui \
    $$PWD/tcphandlerwidget.ui \
    $$PWD/udphandlerwidget.ui

SOURCES += \
    $$PWD/dbhandlerwidget.cpp \
    $$PWD/filehandlerwidget.cpp \
    $$PWD/gatehandlerwidget.cpp \
    $$PWD/modbushandlerwidget.cpp \
    $$PWD/sockhandlerwidget.cpp \
    $$PWD/tcphandlerwidget.cpp \
    $$PWD/udphandlerwidget.cpp
