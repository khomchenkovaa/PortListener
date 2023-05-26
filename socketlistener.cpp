#include "socketlistener.h"
#include "ui_socketlistener.h"

#include "message.h"

#include <QTextCodec>

#include <QMessageBox>
#include <QDebug>

/********************************************************/

SocketListener::SocketListener(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SocketListener),
    m_LocalServer(this)
{
    ui->setupUi(this);
    // configure UI default state
    ui->chkText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    updateStatus();
}

/********************************************************/

SocketListener::~SocketListener()
{
    m_LocalServer.close();
    delete ui;
}

/********************************************************/

void SocketListener::setCodecList(const QList<QTextCodec *> &list)
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, list) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
}

/********************************************************/

void SocketListener::onNewConnection()
{
    auto clientSocket = m_LocalServer.nextPendingConnection();
    connect(clientSocket, &QLocalSocket::readyRead,
           this, &SocketListener::onReadyRead);
    connect(clientSocket, &QLocalSocket::stateChanged,
           this, &SocketListener::onLocalSocketStateChanged);

    ui->textLog->append(QString::number(clientSocket->socketDescriptor()) + " connected to server !\n");
    ui->textLog->moveCursor(QTextCursor::End);
}

/********************************************************/

void SocketListener::onLocalSocketStateChanged(QLocalSocket::LocalSocketState  socketState)
{
    if (socketState == QLocalSocket::UnconnectedState) {
        QLocalSocket* clientSocket = static_cast<QLocalSocket*>(QObject::sender());
        ui->textLog->append(QString::number(clientSocket->socketDescriptor()) + " disconnected from server !\n");
        ui->textLog->moveCursor(QTextCursor::End);
        clientSocket->deleteLater();
    }
}

/********************************************************/

void SocketListener::onReadyRead()
{
    QLocalSocket* sender = static_cast<QLocalSocket*>(QObject::sender());
    QByteArray data = sender->readAll();
    QByteArray replyData = processData(sender->socketDescriptor(), data);
    if (!replyData.isEmpty()) {
        sender->write(replyData);
    }
}

/********************************************************/

void SocketListener::on_btnConnect_clicked()
{
    QString socketName = ui->editSocket->text();
    if (m_LocalServer.listen(socketName)) {
        connect(&m_LocalServer, &QLocalServer::newConnection,
                this, &SocketListener::onNewConnection);
    } else {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Socket %1 connection error!\n%2")
                              .arg(socketName)
                              .arg(m_LocalServer.errorString()));
    }
    updateStatus();
}

/********************************************************/

void SocketListener::on_btnDisconnect_clicked()
{
    m_LocalServer.close();
    updateStatus();
}

/********************************************************/

void SocketListener::on_cmbReplyType_currentIndexChanged(int index)
{
    switch (index) {
    case ReplyType::NoReply:
    case ReplyType::EchoReply:
        ui->editReply->setVisible(false);
        break;
    case ReplyType::TextReply:
    case ReplyType::BinaryReply:
        ui->editReply->setVisible(true);
        break;
    }
}

/********************************************************/

void SocketListener::updateStatus()
{
    if (m_LocalServer.isListening()) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the Socket</font>"));
        ui->editSocket->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        emit tabText(QString("Socket [%1]").arg(ui->editSocket->text()));
    } else {
        ui->lblConnection->setText(tr("Socket to listen"));
        ui->editSocket->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        emit tabText(QString("Socket [-]"));
    }
}

/********************************************************/

QByteArray SocketListener::processData(quintptr socketDescriptor, const QByteArray &data)
{
    int mib = ui->cmbCodec->itemData(ui->cmbCodec->currentIndex()).toInt();
    const QTextCodec *codec = QTextCodec::codecForMib(mib);
    const QString name = QLatin1String(codec->name());
    QTextCodec::ConverterState state;

    QString displayData;
    if (ui->chkText->isChecked()) {
        displayData = codec->toUnicode(data.constData(), data.size(), &state);
    } else {
        displayData = QString::fromLatin1(data.toHex());
    }

    // log payload data
    ui->textLog->append(QString("%1 -> %2")
                        .arg(QString::number(socketDescriptor))
                        .arg(displayData));
    ui->textLog->moveCursor(QTextCursor::End);
    // make reply
    switch (ui->cmbReplyType->currentIndex()) {
    case ReplyType::NoReply:
        break;
    case ReplyType::EchoReply:
        return data;
    case ReplyType::TextReply:
        return codec->fromUnicode(ui->editReply->text());
    case ReplyType::BinaryReply:
        return QByteArray::fromHex(ui->editReply->text().toLatin1());
    }
    return QByteArray();
}

/********************************************************/
