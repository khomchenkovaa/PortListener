INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dbhandler.h \
    $$PWD/dbhandlerconf.h \
    $$PWD/dbhandlerwidget.h \
    $$PWD/filehandler.h \
    $$PWD/filehandlerwidget.h \
    $$PWD/iodecoder.h

SOURCES += \
    $$PWD/dbhandler.cpp \
    $$PWD/dbhandlerwidget.cpp \
    $$PWD/filehandler.cpp \
    $$PWD/filehandlerwidget.cpp \
    $$PWD/iodecoder.cpp

FORMS += \
    $$PWD/dbhandlerwidget.ui \
    $$PWD/filehandlerwidget.ui