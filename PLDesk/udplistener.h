#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include "listenerwidget.h"

#include <QUdpSocket>

namespace Ui {
class UdpListener;
}

class UdpListener : public ListenerWidget
{
    Q_OBJECT

public:
    explicit UdpListener(QWidget *parent = nullptr);
    ~UdpListener();

signals:
    void tabText(const QString &label);

public slots:
    void readPendingDatagrams();

private slots:
    void doConnect();
    void doDisconnect();
    void onInputFormatChanged();
    void changeReplyType(int index);
    void changeHandler(int index);

private:
    void updateStatus();
    void updateCodecs();
    QByteArray processData(const QHostAddress &host, const QByteArray &data);

private:
    Ui::UdpListener *ui;
    QUdpSocket      *m_UdpSocket;
};

#endif // UDPLISTENER_H
