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

    enum ReplyType {
        NoReply,
        EchoReply,
        TextReply,
        BinaryReply
    };

public:
    explicit UdpListener(QWidget *parent = nullptr);
    ~UdpListener();

    void setCodecList(const QList<QTextCodec *> &list);

signals:
    void tabText(const QString &label);

public slots:
    void readPendingDatagrams();

private slots:
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();
    void on_chkText_stateChanged(int arg1);
    void on_cmbReplyType_currentIndexChanged(int index);

private:
    void updateStatus();
    QByteArray processData(const QHostAddress &host, const QByteArray &data);

private:
    Ui::UdpListener *ui;
    QUdpSocket *m_UdpSocket;
};

#endif // UDPLISTENER_H
