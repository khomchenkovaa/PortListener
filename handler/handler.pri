INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dbhandler.h \
    $$PWD/dbhandlerconf.h \
    $$PWD/filehandler.h \
    $$PWD/gatehandler.h \
    $$PWD/gatehandlerconf.h \
    $$PWD/iodecoder.h \
    $$PWD/sockhandler.h \
    $$PWD/tcphandler.h \
    $$PWD/udphandler.h

SOURCES += \
    $$PWD/dbhandler.cpp \
    $$PWD/filehandler.cpp \
    $$PWD/gatehandler.cpp \
    $$PWD/iodecoder.cpp \
    $$PWD/sockhandler.cpp \
    $$PWD/tcphandler.cpp \
    $$PWD/udphandler.cpp