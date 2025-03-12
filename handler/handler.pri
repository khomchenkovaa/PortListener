INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dbhandler.h \
    $$PWD/dbhandlerconf.h \
    $$PWD/dephandler.h \
    $$PWD/dephandlerconf.h \
    $$PWD/filehandler.h \
    $$PWD/iodecoder.h \
    $$PWD/modbusclientconf.h \
    $$PWD/modbushandler.h \
    $$PWD/modbushandlerconf.h \
    $$PWD/modbushelper.h \
    $$PWD/sockhandler.h \
    $$PWD/tcphandler.h \
    $$PWD/udphandler.h

SOURCES += \
    $$PWD/dbhandler.cpp \
    $$PWD/dephandler.cpp \
    $$PWD/filehandler.cpp \
    $$PWD/iodecoder.cpp \
    $$PWD/modbushandler.cpp \
    $$PWD/sockhandler.cpp \
    $$PWD/tcphandler.cpp \
    $$PWD/udphandler.cpp