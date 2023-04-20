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

private:
    void setupUI();
    void findCodecs();

private:
    Ui::MainWindow *ui;
    QList<QTextCodec *> codecs;

};
#endif // MAINWINDOW_H
