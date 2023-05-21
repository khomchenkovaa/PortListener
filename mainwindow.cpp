#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "tcplistener.h"
#include "udplistener.h"
#include "socketlistener.h"

#include <QTextCodec>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

/********************************************************/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUI();
    findCodecs();
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
    widget->setCodecList(codecs);
    connect(widget, &TcpListener::tabText, [this, widget](const QString &label){
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
    widget->setCodecList(codecs);
    connect(widget, &UdpListener::tabText, [this, widget](const QString &label){
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
    widget->setCodecList(codecs);
    connect(widget, &SocketListener::tabText, [this, widget](const QString &label){
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
    connect(ui->btnSocket, &QPushButton::clicked,
            ui->actionSocket, &QAction::triggered);

    connect(ui->actionTCP_port, &QAction::triggered,
            this, &MainWindow::addTcpListener);
    connect(ui->actionUDP_port, &QAction::triggered,
            this, &MainWindow::addUdpListener);
    connect(ui->actionSocket, &QAction::triggered,
            this, &MainWindow::addSocketListener);
}

/********************************************************/

void MainWindow::findCodecs()
{
    QMap<QString, QTextCodec *> codecMap;
    QRegularExpression iso8859RegExp("^ISO[- ]8859-([0-9]+).*$");
    QRegularExpressionMatch match;

    foreach (int mib, QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        QString sortKey = codec->name().toUpper();
        int rank;

        if (sortKey.startsWith(QLatin1String("UTF-8"))) {
            rank = 1;
        } else if (sortKey.startsWith(QLatin1String("UTF-16"))) {
            rank = 2;
        } else if ((match = iso8859RegExp.match(sortKey)).hasMatch()) {
            if (match.captured(1).size() == 1)
                rank = 3;
            else
                rank = 4;
        } else {
            rank = 5;
        }
        sortKey.prepend(QLatin1Char('0' + rank));

        codecMap.insert(sortKey, codec);
    }
    codecs = codecMap.values();
}

/********************************************************/
