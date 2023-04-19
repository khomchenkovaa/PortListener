#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onNewConnection();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onReadyRead();

private slots:
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();

private:
    void updateStatus();

private:
    Ui::MainWindow *ui;
    QTcpServer      m_TcpServer;
};
#endif // MAINWINDOW_H
