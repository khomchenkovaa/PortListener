#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "tcplistener.h"
#include "udplistener.h"
#include "socketlistener.h"

#ifdef QT_SERIALBUS_LIB
#include "modbustcplistener.h"
#endif

#include <QTabBar>

/********************************************************/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUI();
}

/********************************************************/

MainWindow::~MainWindow()
{
    delete ui;
}

/********************************************************/

void MainWindow::addTcpListener()
{
    auto widget = new TcpListener(this);
    ui->tabWidget->addTab(widget, tr("TCP [-]"));
    ui->tabWidget->setCurrentWidget(widget);
    connect(widget, &TcpListener::tabText, this, [this, widget](const QString &label){
        int idx = ui->tabWidget->indexOf(widget);
        ui->tabWidget->setTabText(idx, label);
    });
}

/********************************************************/

void MainWindow::addUdpListener()
{
    auto widget = new UdpListener(this);
    ui->tabWidget->addTab(widget, tr("UDP [-]"));
    ui->tabWidget->setCurrentWidget(widget);
    connect(widget, &UdpListener::tabText, this, [this, widget](const QString &label){
        int idx = ui->tabWidget->indexOf(widget);
        ui->tabWidget->setTabText(idx, label);
    });
}

/********************************************************/

void MainWindow::addModbusTcpListener()
{
#ifdef QT_SERIALBUS_LIB
    auto widget = new ModbusTcpListener(this);
    ui->tabWidget->addTab(widget, tr("Modbus TCP [-]"));
    ui->tabWidget->setCurrentWidget(widget);
    connect(widget, &ModbusTcpListener::tabText, this, [this, widget](const QString &label){
        int idx = ui->tabWidget->indexOf(widget);
        ui->tabWidget->setTabText(idx, label);
    });
#endif
}

/********************************************************/

void MainWindow::addSocketListener()
{
    auto widget = new SocketListener(this);
    ui->tabWidget->addTab(widget, tr("Socket [-]"));
    ui->tabWidget->setCurrentWidget(widget);
    connect(widget, &SocketListener::tabText, this, [this, widget](const QString &label){
        int idx = ui->tabWidget->indexOf(widget);
        ui->tabWidget->setTabText(idx, label);
    });
}

/********************************************************/

void MainWindow::setupUI()
{
    connect(ui->btnTcp, &QPushButton::clicked,
            ui->actionTCP_port, &QAction::triggered);
    connect(ui->btnUdp, &QPushButton::clicked,
            ui->actionUDP_port, &QAction::triggered);
    connect(ui->btnModbus, &QPushButton::clicked,
            ui->actionModbus_TCP, &QAction::triggered);
    connect(ui->btnSocket, &QPushButton::clicked,
            ui->actionSocket, &QAction::triggered);

    connect(ui->actionTCP_port, &QAction::triggered,
            this, &MainWindow::addTcpListener);
    connect(ui->actionUDP_port, &QAction::triggered,
            this, &MainWindow::addUdpListener);
    connect(ui->actionModbus_TCP, &QAction::triggered,
            this, &MainWindow::addModbusTcpListener);
    connect(ui->actionSocket, &QAction::triggered,
            this, &MainWindow::addSocketListener);

#ifndef QT_SERIALBUS_LIB
    ui->btnModbus->setDisabled(true);
    ui->actionModbus_TCP->setDisabled(true);
#endif

    connect(ui->tabWidget->tabBar(), &QTabBar::tabCloseRequested, this, [this](int index){
        QWidget *widget = ui->tabWidget->widget(index);
        ui->tabWidget->removeTab(index);
        widget->deleteLater();
    });

}

/********************************************************/
