#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "listenerwidget.h"

#include <QTcpSocket>

namespace Ui {
class TcpClient;
}

class TcpClient : public ListenerWidget
{
    Q_OBJECT

public:
    explicit TcpClient(QWidget *parent = nullptr);
    ~TcpClient();

signals:
    void tabText(const QString &label);

public Q_SLOTS:
    void onReadyRead();

protected:
    QTextBrowser *textLog() const;

private Q_SLOTS:
    void doConnect();
    void doDisconnect();
    void onInputFormatChanged();
    void changeDecoder(int index);
    void changeReplyType(int index);
    void changeHandler(int index);

private:
    /// configure UI default state
    void setupUiDefaultState();
    void updateStatus();
    void updateCodecs();
    QByteArray processData(const QHostAddress &host, const QByteArray &data);

private:
    Ui::TcpClient *ui;
    QTcpSocket     *m_TcpSocket;
};

#endif // TCPCLIENT_H
