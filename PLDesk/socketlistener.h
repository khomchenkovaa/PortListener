#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

#include "listenerwidget.h"

#include <QLocalServer>
#include <QLocalSocket>

namespace Ui {
class SocketListener;
}

class SocketListener : public ListenerWidget
{
    Q_OBJECT

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
    void doConnect();
    void doDisconnect();
    void onInputFormatChanged();
    void changeReplyType(int index);
    void changeHandler(int index);
    void printInfo(const QString& host, const QString& msg);
    void printMessage(const QString& host, const QString& msg);
    void printError(const QString& host, const QString& msg);

private:
    /// configure UI default state
    void setupUiDefaultState();
    void updateStatus();
    void updateCodecs();
    QByteArray processData(quintptr socketDescriptor, const QByteArray &data);

private:
    Ui::SocketListener *ui;
    QLocalServer m_LocalServer;
};

#endif // SOCKETLISTENER_H
