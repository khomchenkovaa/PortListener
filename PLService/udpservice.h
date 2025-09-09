#ifndef UDP_SERVICE_H
#define UDP_SERVICE_H

#include "udpdaemon.h"

#include <QCoreApplication>

class UdpService: public QtService<QCoreApplication>
{

public:
    UdpService(int argc, char **argv);

protected:
    void start();
    void pause();
    void resume();

private:
    UdpDaemon *daemon;
};

#endif // UDP_SERVICE_H
