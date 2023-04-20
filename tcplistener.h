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

    enum ReplyType {
        NoReply,
        EchoReply,
        TextReply,
        BinaryReply
    };

public:
    explicit TcpListener(QWidget *parent = nullptr);
    ~TcpListener();

    void setCodecList(const QList<QTextCodec *> &list);

signals:
    void tabText(const QString &label);

public slots:
    void onNewConnection();
    void onTcpSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onReadyRead();

private slots:
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();
    void on_cmbReplyType_currentIndexChanged(int index);

private:
    void updateStatus();
    QByteArray processData(const QHostAddress &host, const QByteArray &data);

private:
    Ui::TcpListener *ui;
    QTcpServer m_TcpServer;
};

#endif // TCPLISTENER_H
