#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include <QWidget>
#include <QTcpServer>

namespace Ui {
class TcpListener;
}

class TcpListener : public QWidget
{
    Q_OBJECT

public:
    explicit TcpListener(QWidget *parent = nullptr);
    ~TcpListener();

signals:
    void tabText(const QString &label);

public slots:
    void onNewConnection();
    void onTcpSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onReadyRead();

private slots:
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();

private:
    void updateStatus();

private:
    Ui::TcpListener *ui;
    QTcpServer m_TcpServer;
};

#endif // TCPLISTENER_H
