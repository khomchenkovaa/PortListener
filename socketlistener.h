#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

#include "iodecoder.h"

#include <QWidget>
#include <QLocalServer>
#include <QLocalSocket>

namespace Ui {
class SocketListener;
}

class SocketListener : public QWidget
{
    Q_OBJECT

    enum ReplyType {
        NoReply,
        EchoReply,
        TextReply,
        BinaryReply
    };

public:
    explicit SocketListener(QWidget *parent = nullptr);
    ~SocketListener();

signals:
    void tabText(const QString &label);

public slots:
    void onNewConnection();
    void onLocalSocketStateChanged(QLocalSocket::LocalSocketState socketState);
    void onReadyRead();

private slots:
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();
    void on_cmbReplyType_currentIndexChanged(int index);

private:
    void updateStatus();
    void updateCodecs();
    QByteArray processData(quintptr socketDescriptor, const QByteArray &data);

private:
    Ui::SocketListener *ui;
    QLocalServer m_LocalServer;
    IODecoder  ioDecoder;
};

#endif // SOCKETLISTENER_H
