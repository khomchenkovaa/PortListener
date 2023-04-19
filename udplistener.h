#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include <QWidget>
#include <QUdpSocket>

namespace Ui {
class UdpListener;
}

class UdpListener : public QWidget
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
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();

private:
    void updateStatus();

private:
    Ui::UdpListener *ui;
    QUdpSocket *m_UdpSocket;
};

#endif // UDPLISTENER_H
