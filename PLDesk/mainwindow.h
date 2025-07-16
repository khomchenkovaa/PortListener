#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void addTcpListener();
    void addUdpListener();
    void addSocketListener();
    void addMqueueListener();
    void addModbusTcpServer();
    void addModbusTcpClient();
    void addDataSender();

private:
    void setupUI();

private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
