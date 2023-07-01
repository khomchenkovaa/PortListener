#ifndef QTDAEMON_H
#define QTDAEMON_H

#include "tcpdaemon.h"

#include <QCoreApplication>

class TcpService: public QtService<QCoreApplication> {

public:
    TcpService(int argc, char **argv);

protected:
    void start();
    void pause();
    void resume();

private:
    TcpDaemon *daemon;
};

#endif // QTDAEMON_H
