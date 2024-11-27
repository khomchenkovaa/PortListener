INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dbhandler.h \
    $$PWD/dbhandlerconf.h \
    $$PWD/filehandler.h \
    $$PWD/gatehandler.h \
    $$PWD/gatehandlerconf.h \
    $$PWD/iodecoder.h \
    $$PWD/modbusclientconf.h \
    $$PWD/modbushandler.h \
    $$PWD/modbushandlerconf.h \
    $$PWD/sockhandler.h \
    $$PWD/tcphandler.h \
    $$PWD/udphandler.h

SOURCES += \
    $$PWD/dbhandler.cpp \
    $$PWD/filehandler.cpp \
    $$PWD/gatehandler.cpp \
    $$PWD/iodecoder.cpp \
    $$PWD/modbushandler.cpp \
    $$PWD/sockhandler.cpp \
    $$PWD/tcphandler.cpp \
    $$PWD/udphandler.cpp