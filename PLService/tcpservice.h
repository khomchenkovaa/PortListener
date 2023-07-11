#ifndef TCP_SERVICE_H
#define TCP_SERVICE_H

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

#endif // TCP_SERVICE_H
