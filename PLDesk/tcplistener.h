#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include "listenerwidget.h"

#include <QTcpServer>

namespace Ui {
class TcpListener;
}

class TcpListener : public ListenerWidget
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
    Ui::TcpListener *ui;
    QTcpServer m_TcpServer;
};

#endif // TCPLISTENER_H
