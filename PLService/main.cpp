#include "tcpservice.h"
#include "udpservice.h"

#include "settings.h"

#include <QDir>
#include <QSettings>

/*
 * Command line arguments
 * -i (-install) Install the service.
 * -u (-uninstall) Uninstall the service.
 * -e (-exec) Execute the service as a standalone application (useful for debug purposes). This is a blocking call, the service will be executed like a normal application. In this mode you will not be able to communicate with the service from the contoller.
 * -t (-terminate) Stop the service.
 * -p (-pause) Pause the service.
 * -r (-resume) Resume a paused service.
 * -c cmd (-command cmd) Send the user defined command code cmd to the service application.
 * -v (-version) Display version and status information.
 * Without arguments start the service
 */

int main(int argc, char *argv[]) {
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);

#if !defined(Q_OS_WIN)
    // QtService stores service settings in SystemScope, which normally require root privileges.
    // To allow testing this example as non-root, we change the directory of the SystemScope settings file.
    QSettings::setPath(QSettings::NativeFormat, QSettings::SystemScope, QDir::tempPath());
    qWarning("(Version uses dummy settings file: %s/QtSoftware.conf)", QDir::tempPath().toLatin1().constData());
#endif
//    TcpService service(argc, argv);
    UdpService service(argc, argv);
    return service.exec();
}
