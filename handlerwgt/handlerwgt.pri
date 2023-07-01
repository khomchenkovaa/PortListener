INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dbhandlerwidget.h \
    $$PWD/filehandlerwidget.h \
    $$PWD/messagehandlerwgt.h \
    $$PWD/sockhandlerwidget.h \
    $$PWD/tcphandlerwidget.h \
    $$PWD/udphandlerwidget.h

FORMS += \
    $$PWD/dbhandlerwidget.ui \
    $$PWD/filehandlerwidget.ui \
    $$PWD/sockhandlerwidget.ui \
    $$PWD/tcphandlerwidget.ui \
    $$PWD/udphandlerwidget.ui

SOURCES += \
    $$PWD/dbhandlerwidget.cpp \
    $$PWD/filehandlerwidget.cpp \
    $$PWD/sockhandlerwidget.cpp \
    $$PWD/tcphandlerwidget.cpp \
    $$PWD/udphandlerwidget.cpp
