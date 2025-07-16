# unix:!macx: LIBS += -L$$PWD/lib64/ -lrt
# depends on libs/base

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/mq.h \
    $$PWD/mqworker.h

SOURCES += \
    $$PWD/mq.cpp \
    $$PWD/mqworker.cpp
