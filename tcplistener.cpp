#include "tcplistener.h"
#include "ui_tcplistener.h"

#include <QHostAddress>
#include <QTcpSocket>
#include <QTextCodec>

#include <QMessageBox>
#include <QDebug>

/********************************************************/

TcpListener::TcpListener(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcpListener),
    m_TcpServer(this)
{
    ui->setupUi(this);
    // configure UI default state
    ui->chkText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    updateStatus();
}

/********************************************************/

TcpListener::~TcpListener()
{
    m_TcpServer.close();
    delete ui;
}

/********************************************************/

void TcpListener::setCodecList(const QList<QTextCodec *> &list)
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, list) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
}

/********************************************************/

void TcpListener::onNewConnection()
{
    auto clientSocket = m_TcpServer.nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead,
           this, &TcpListener::onReadyRead);
    connect(clientSocket, &QTcpSocket::stateChanged,
           this, &TcpListener::onTcpSocketStateChanged);

    ui->textLog->append(clientSocket->peerAddress().toString() + " connected to server !\n");
    ui->textLog->moveCursor(QTextCursor::End);
}

/********************************************************/

void TcpListener::onTcpSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState) {
        QTcpSocket* clientSocket = static_cast<QTcpSocket*>(QObject::sender());
        ui->textLog->append(clientSocket->peerAddress().toString() + " disconnected from server !\n");
        ui->textLog->moveCursor(QTextCursor::End);
        clientSocket->deleteLater();
    }
}

/********************************************************/

void TcpListener::onReadyRead()
{
    QTcpSocket* sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray data = sender->readAll();
    QByteArray replyData = processData(sender->peerAddress(), data);
    if (!replyData.isEmpty()) {
        sender->write(replyData);
    }
}

/********************************************************/

void TcpListener::on_btnConnect_clicked()
{
    quint16 port = ui->spinPort->value();
    if (m_TcpServer.listen(QHostAddress::Any, port)) {
        connect(&m_TcpServer, &QTcpServer::newConnection,
                this, &TcpListener::onNewConnection);
    } else {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("TCP Port %1 connection error!\n%2")
                              .arg(port)
                              .arg(m_TcpServer.errorString()));
    }
    updateStatus();
}

/********************************************************/

void TcpListener::on_btnDisconnect_clicked()
{
    m_TcpServer.close();
    updateStatus();
}

/********************************************************/

void TcpListener::on_cmbReplyType_currentIndexChanged(int index)
{
    ui->editReply->setVisible(index == ReplyType::PredefinedReply);
}

/********************************************************/

void TcpListener::updateStatus()
{
    if (m_TcpServer.isListening()) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the TCP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        emit tabText(QString("TCP [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("Choose TCP port to listen"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        emit tabText(QString("TCP [-]"));
    }
}

/********************************************************/

QByteArray TcpListener::processData(const QHostAddress &host, const QByteArray &data)
{
    QString displayData;
    if (ui->chkText->isChecked()) {
        int mib = ui->cmbCodec->itemData(ui->cmbCodec->currentIndex()).toInt();
        const QTextCodec *codec = QTextCodec::codecForMib(mib);
        const QString name = QLatin1String(codec->name());
        QTextCodec::ConverterState state;
        displayData = codec->toUnicode(data.constData(), data.size(), &state);
    } else {
        displayData = QString::fromStdString(data.toHex().toStdString());
    }

    // log payload data
    ui->textLog->append(QString("%1 -> %2")
                        .arg(host.toString())
                        .arg(displayData));
    ui->textLog->moveCursor(QTextCursor::End);
    // make reply
    switch (ui->cmbReplyType->currentIndex()) {
    case ReplyType::NoReply:
        break;
    case ReplyType::EchoReply:
        return data;
    case ReplyType::PredefinedReply:
        return ui->editReply->text().toUtf8();
    }
    return QByteArray();
}

/********************************************************/
