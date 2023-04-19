#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QHostAddress>
#include <QAbstractSocket>

#include <QMessageBox>

/********************************************************/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_TcpServer(this)
{
    ui->setupUi(this);
    updateStatus();
}

/********************************************************/

MainWindow::~MainWindow()
{
    delete ui;
}

/********************************************************/

void MainWindow::onNewConnection()
{
   auto clientSocket = m_TcpServer.nextPendingConnection();
   connect(clientSocket, &QTcpSocket::readyRead,
           this, &MainWindow::onReadyRead);
   connect(clientSocket, &QTcpSocket::stateChanged,
           this, &MainWindow::onSocketStateChanged);

    ui->textLog->append(clientSocket->peerAddress().toString() + " connected to server !\n");
    ui->textLog->moveCursor(QTextCursor::End);
}

/********************************************************/

void MainWindow::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState) {
        QTcpSocket* clientSocket = static_cast<QTcpSocket*>(QObject::sender());
        ui->textLog->append(clientSocket->peerAddress().toString() + " disconnected from server !\n");
        ui->textLog->moveCursor(QTextCursor::End);
        clientSocket->deleteLater();
    }
}

/********************************************************/

void MainWindow::onReadyRead()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray data = sender->readAll();
    ui->textLog->append(QString(data) + "\n");
    ui->textLog->moveCursor(QTextCursor::End);
}

/********************************************************/

void MainWindow::on_btnConnect_clicked()
{
    quint16 port = ui->spinPort->value();
    if (m_TcpServer.listen(QHostAddress::Any, port)) {
        connect(&m_TcpServer, &QTcpServer::newConnection,
                this, &MainWindow::onNewConnection);
        updateStatus();
    } else {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Port %1 connection error!\n%2")
                              .arg(port)
                              .arg(m_TcpServer.errorString()));
    }

}

/********************************************************/

void MainWindow::on_btnDisconnect_clicked()
{
    m_TcpServer.close();
    updateStatus();
}

/********************************************************/

void MainWindow::updateStatus()
{
    if (m_TcpServer.isListening()) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the TCP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
    } else {
        ui->lblConnection->setText(tr("Choose TCP port to listen"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
    }
}

/********************************************************/
