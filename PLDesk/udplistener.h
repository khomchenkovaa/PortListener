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

public Q_SLOTS:
    void readPendingDatagrams();

protected:
    QTextBrowser *textLog() const;

private Q_SLOTS:
    void doConnect();
    void doDisconnect();
    void onInputFormatChanged();
    void changeReplyType(int index);
    void changeHandler(int index);

private:
    /// configure UI default state
    void setupUiDefaultState();
    void updateStatus();
    void updateCodecs();
    QByteArray processData(const QHostAddress &host, const QByteArray &data);

private:
    Ui::UdpListener *ui;
    QUdpSocket      *m_UdpSocket;
};

#endif // UDPLISTENER_H
