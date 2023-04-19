#include "udplistener.h"
#include "ui_udplistener.h"

#include <QNetworkDatagram>

#include <QMessageBox>

/********************************************************/

UdpListener::UdpListener(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UdpListener),
    m_UdpSocket(Q_NULLPTR)
{
    ui->setupUi(this);
    updateStatus();
}

/********************************************************/

UdpListener::~UdpListener()
{
    if (m_UdpSocket) {
        m_UdpSocket->close();
    }
    delete ui;
}

/********************************************************/

void UdpListener::readPendingDatagrams()
{
    while (m_UdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_UdpSocket->receiveDatagram();
        ui->textLog->append(QString("%1 -> %2")
                            .arg(datagram.senderAddress().toString())
                            .arg(QString(datagram.data())));
        ui->textLog->moveCursor(QTextCursor::End);
    }
}

/********************************************************/

void UdpListener::on_btnConnect_clicked()
{
    quint16 port = ui->spinPort->value();
    m_UdpSocket = new QUdpSocket(this);
    if (m_UdpSocket->bind(QHostAddress::Any, port)) {
        connect(m_UdpSocket, &QUdpSocket::readyRead,
               this, &UdpListener::readPendingDatagrams);
    } else {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("UDP Port %1 bind error!\n%2")
                              .arg(port)
                              .arg(m_UdpSocket->errorString()));
        m_UdpSocket->deleteLater();
        m_UdpSocket = Q_NULLPTR;
    }
    updateStatus();
}

/********************************************************/

void UdpListener::on_btnDisconnect_clicked()
{
    if (m_UdpSocket) {
        m_UdpSocket->close();
        m_UdpSocket->deleteLater();
        m_UdpSocket = Q_NULLPTR;
    }
    updateStatus();
}

/********************************************************/

void UdpListener::updateStatus()
{
    if (m_UdpSocket) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the UDP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        emit tabText(QString("UDP [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("Choose UDP port to listen"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        emit tabText(QString("UDP [-]"));
    }
}

/********************************************************/
