QT += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# comment next line if you don't want the modbus
QT += serialbus

unix:!macx: DEFINES += MQUEUE

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include("../common/ext/ext.pri")
include("../common/depprotocol/depprotocol.pri")
include("../messaging/messaging.pri")
include("../handler/handler.pri")
include("../handlerwgt/handlerwgt.pri")

SOURCES += \
    datasender.cpp \
    listenerwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    socketlistener.cpp \
    tcplistener.cpp \
    udplistener.cpp

HEADERS += \
    datasender.h \
    listenerwidget.h \
    mainwindow.h \
    socketlistener.h \
    tcplistener.h \
    udplistener.h

FORMS += \
    datasender.ui \
    mainwindow.ui \
    socketlistener.ui \
    tcplistener.ui \
    udplistener.ui

contains(QT, serialbus) {
    SOURCES += \
        modbustcpclient.cpp \
        modbustcplistener.cpp

    HEADERS += \
        modbustcpclient.h \
        modbustcplistener.h

    FORMS += \
        modbustcpclient.ui \
        modbustcplistener.ui
}

contains(DEFINES, MQUEUE) {
    include("../common/mq/mq.pri")

    SOURCES += \
        mqueuelistener.cpp

    HEADERS += \
        mqueuelistener.h

    FORMS += \
        mqueuelistener.ui

    LIBS += -lrt
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../PortListener.qrc
