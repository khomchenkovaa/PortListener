#include "socketlistener.h"
#include "ui_socketlistener.h"

#include "iodecoder.h"

#include "filehandler.h"
#include "dbhandler.h"
#include "udphandler.h"
#include "tcphandler.h"
#include "sockhandler.h"

#include "filehandlerwidget.h"
#include "dbhandlerwidget.h"
#include "sockhandlerwidget.h"
#include "tcphandlerwidget.h"
#include "udphandlerwidget.h"

#include <QTextCodec>

#include <QMessageBox>

#include <QDebug>

/********************************************************/

SocketListener::SocketListener(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SocketListener),
    m_LocalServer(this),
    m_Handler(Q_NULLPTR)
{
    ui->setupUi(this);
    // configure UI default state
    ui->rbBinary->setChecked(false);
    ui->rbText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);

    connect(ui->rbBinary, &QRadioButton::toggled,
            this, &SocketListener::onInputFormatChanged);
    connect(ui->rbText, &QRadioButton::toggled,
            this, &SocketListener::onInputFormatChanged);

    updateStatus();
    updateCodecs();
}

/********************************************************/

SocketListener::~SocketListener()
{
    m_LocalServer.close();
    delete ui;
}

/********************************************************/

void SocketListener::onNewConnection()
{
    auto clientSocket = m_LocalServer.nextPendingConnection();
    if (!clientSocket) return;

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
    if (m_LocalServer.isListening() && m_Handler) {
        MessageHandlerWgt *editor = findChild<MessageHandlerWgt*>();
        if (editor) {
            m_Handler->setSettings(editor->settings());
        }
        m_Handler->doConnect(ui->rbBinary->isChecked());
        if (m_Handler->hasError()) {
            ui->textLog->append(QString("%1 : %2")
                                .arg(m_Handler->name())
                                .arg(m_Handler->lastError()));
        }
    }
    updateStatus();
}

/********************************************************/

void SocketListener::on_btnDisconnect_clicked()
{
    m_LocalServer.close();
    if (m_Handler) {
        m_Handler->doDisconnect();
    }
    updateStatus();
}

/********************************************************/

void SocketListener::onInputFormatChanged()
{
    ui->cmbCodec->setVisible(ui->rbText->isChecked());
}

/********************************************************/

void SocketListener::on_cmbReplyType_currentIndexChanged(int index)
{
    switch (index) {
    case ReplyType::NoReply:
    case ReplyType::EchoReply:
    case ReplyType::ActionReply:
        ui->editReply->setVisible(false);
        break;
    case ReplyType::TextReply:
    case ReplyType::BinaryReply:
        ui->editReply->setVisible(true);
        break;
    }
}

/********************************************************/

void SocketListener::on_cmbHandler_currentIndexChanged(int index)
{
    MessageHandlerWgt *editor = findChild<MessageHandlerWgt*>();
    if (editor) {
        editor->deleteLater();
    }
    if (m_Handler) {
        m_Handler->deleteLater();
        m_Handler = Q_NULLPTR;
    }
    switch (index) {
    case ActionHandler::NoActionHandler:
        break;
    case ActionHandler::FileActionHandler:
        m_Handler = new FileHandler(this);
        editor = new FileHandlerWidget(this);
        break;
    case ActionHandler::DbActionHandler:
        m_Handler = new DbHandler(this);
        editor = new DbHandlerWidget(this);
        break;
    case ActionHandler::UdpActionHandler:
        m_Handler = new UdpHandler(this);
        editor = new UdpHandlerWidget(this);
        break;
    case ActionHandler::TcpActionHandler:
        m_Handler = new TcpHandler(this);
        editor = new TcpHandlerWidget(this);
        break;
    case ActionHandler::SocketActionHandler:
        m_Handler = new SockHandler(this);
        editor = new SockHandlerWidget(this);
        break;
    }
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
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
        ui->boxAction->setEnabled(false);
        emit tabText(QString("Socket [%1]").arg(ui->editSocket->text()));
    } else {
        ui->lblConnection->setText(tr("Socket to listen"));
        ui->editSocket->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxAction->setEnabled(true);
        emit tabText(QString("Socket [-]"));
    }
}

/********************************************************/

void SocketListener::updateCodecs()
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, IODecoder::findCodecs()) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
}

/********************************************************/

QByteArray SocketListener::processData(quintptr socketDescriptor, const QByteArray &data)
{
    int mib = ui->cmbCodec->itemData(ui->cmbCodec->currentIndex()).toInt();
    ioDecoder.setMib(mib);
    QString displayData = ioDecoder.toUnicode(data, ui->rbBinary->isChecked());

    // log payload data
    ui->textLog->append(QString("%1 -> %2")
                        .arg(QString::number(socketDescriptor))
                        .arg(displayData));
    ui->textLog->moveCursor(QTextCursor::End);

    QByteArray reply;
    // Handler
    if (m_Handler) {
        if (ui->rbText->isChecked()) {
            reply = m_Handler->processData(displayData);
        } else {
            reply = m_Handler->processData(data);
        }
        if (m_Handler->hasError()) {
            ui->textLog->append(QString("%1 -> %2")
                                .arg(QString::number(socketDescriptor))
                                .arg(m_Handler->lastError()));
            ui->textLog->moveCursor(QTextCursor::End);
        }
    }

    // make reply
    switch (ui->cmbReplyType->currentIndex()) {
    case ReplyType::NoReply:
        reply.clear();
        break;
    case ReplyType::EchoReply:
        reply = data;
        break;
    case ReplyType::TextReply:
        reply = ioDecoder.fromUnicode(ui->editReply->text());
        break;
    case ReplyType::BinaryReply:
        reply = ioDecoder.fromUnicode(ui->editReply->text(), true);
        break;
    }
    return reply;
}

/********************************************************/
