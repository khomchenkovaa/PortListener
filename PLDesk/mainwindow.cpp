#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "tcplistener.h"
#include "udplistener.h"
#include "socketlistener.h"
#include "filereader.h"
#include "datasender.h"

#ifdef QT_SERIALBUS_LIB
#include "modbustcpclient.h"
#include "modbustcplistener.h"
#endif

#ifdef MQUEUE
#include "mqueuelistener.h"
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

void MainWindow::addMqueueListener()
{
#ifdef MQUEUE
    auto widget = new MQueueListener(this);
    ui->tabWidget->addTab(widget, tr("MQ [-]"));
    ui->tabWidget->setCurrentWidget(widget);
    connect(widget, &MQueueListener::tabText, this, [this, widget](const QString &label) {
        int idx = ui->tabWidget->indexOf(widget);
        ui->tabWidget->setTabText(idx, label);
    });
#endif
}

/********************************************************/

void MainWindow::addModbusTcpServer()
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

void MainWindow::addModbusTcpClient()
{
#ifdef QT_SERIALBUS_LIB
    auto widget = new ModbusTcpClient(this);
    ui->tabWidget->addTab(widget, tr("Modbus Client [-]"));
    ui->tabWidget->setCurrentWidget(widget);
    connect(widget, &ModbusTcpClient::tabText, this, [this, widget](const QString &label){
        int idx = ui->tabWidget->indexOf(widget);
        ui->tabWidget->setTabText(idx, label);
    });
#endif
}

/********************************************************/

void MainWindow::addFileReader()
{
    auto widget = new FileReader(this);
    ui->tabWidget->addTab(widget, tr("File [-]"));
    ui->tabWidget->setCurrentWidget(widget);
    connect(widget, &FileReader::tabText, this, [this, widget](const QString &label){
        int idx = ui->tabWidget->indexOf(widget);
        ui->tabWidget->setTabText(idx, label);
    });
}

/********************************************************/

void MainWindow::addDataSender()
{
    auto widget = new DataSender(this);
    ui->tabWidget->addTab(widget, tr("Data Sender"));
    ui->tabWidget->setCurrentWidget(widget);
}

/********************************************************/

void MainWindow::setupUI()
{
    connect(ui->btnTcp, &QPushButton::clicked,
            ui->actionTCP_port, &QAction::triggered);
    connect(ui->btnUdp, &QPushButton::clicked,
            ui->actionUDP_port, &QAction::triggered);
    connect(ui->btnSocket, &QPushButton::clicked,
            ui->actionSocket, &QAction::triggered);
    connect(ui->btnMqueue, &QPushButton::clicked,
            ui->actionMQueue, &QAction::triggered);
    connect(ui->btnModbusTcpServer, &QPushButton::clicked,
            ui->actionModbusTcpServer, &QAction::triggered);
    connect(ui->btnModbusTcpClient, &QPushButton::clicked,
            ui->actionModbusTcpClient, &QAction::triggered);

    connect(ui->actionTCP_port, &QAction::triggered,
            this, &MainWindow::addTcpListener);
    connect(ui->actionUDP_port, &QAction::triggered,
            this, &MainWindow::addUdpListener);
    connect(ui->actionSocket, &QAction::triggered,
            this, &MainWindow::addSocketListener);
    connect(ui->actionMQueue, &QAction::triggered,
            this, &MainWindow::addMqueueListener);
    connect(ui->actionModbusTcpServer, &QAction::triggered,
            this, &MainWindow::addModbusTcpServer);
    connect(ui->actionModbusTcpClient, &QAction::triggered,
            this, &MainWindow::addModbusTcpClient);
    connect(ui->actionDataSender, &QAction::triggered,
            this, &MainWindow::addDataSender);
    connect(ui->actionFileReader, &QAction::triggered,
            this, &MainWindow::addFileReader);

#ifndef QT_SERIALBUS_LIB
    ui->btnModbusTcpServer->setDisabled(true);
    ui->actionModbusTcpServer->setDisabled(true);
    ui->btnModbusTcpClient->setDisabled(true);
    ui->actionModbusTcpClient->setDisabled(true);
#endif

#ifndef MQUEUE
    ui->btnMqueue->setDisabled(true);
    ui->actionMQueue->setDisabled(true);
#endif

    connect(ui->tabWidget->tabBar(), &QTabBar::tabCloseRequested, this, [this](int index){
        QWidget *widget = ui->tabWidget->widget(index);
        ui->tabWidget->removeTab(index);
        widget->deleteLater();
    });

}

/********************************************************/
