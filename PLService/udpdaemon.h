#ifndef UDP_DAEMON_H
#define UDP_DAEMON_H

#include "messagehandler.h"
#include "qtservice.h"

#include <QUdpSocket>

class UdpDaemon: public QObject
{
    Q_OBJECT

public:
    UdpDaemon(quint16 port, QObject* parent = 0);
    ~UdpDaemon();

    bool isListening() const;

public slots:
    void startServer(quint16 port = 2424);
    void stopServer();
    void pauseServer();
    void resumeServer();

private slots:
    void readPendingDatagrams();

private:
    void initHandler();
    QByteArray processData(const QByteArray &data);

private:
    bool disabled;
    QUdpSocket     *m_UdpSocket;
    MessageHandler *m_Handler;
};

#endif // UDP_DAEMON_H
