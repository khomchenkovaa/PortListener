#include "tcplistener.h"
#include "ui_tcplistener.h"

#include "filehandler.h"
#include "dbhandler.h"
#include "udphandler.h"
#include "tcphandler.h"
#include "sockhandler.h"
#include "gatehandler.h"

#include "filehandlerwidget.h"
#include "dbhandlerwidget.h"
#include "sockhandlerwidget.h"
#include "tcphandlerwidget.h"
#include "udphandlerwidget.h"
#include "gatehandlerwidget.h"

#include <QHostAddress>
#include <QTcpSocket>
#include <QTextCodec>

#include <QMessageBox>

#include <QDebug>

/********************************************************/

TcpListener::TcpListener(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcpListener),
    m_TcpServer(this),
    m_Handler(Q_NULLPTR)
{
    ui->setupUi(this);

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &TcpListener::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &TcpListener::doDisconnect);
    connect(ui->cmbReplyType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TcpListener::changeReplyType);
    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TcpListener::changeHandler);
    // configure UI default state
    ui->rbBinary->setChecked(false);
    ui->rbText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);

    connect(ui->rbBinary, &QRadioButton::toggled,
            this, &TcpListener::onInputFormatChanged);
    connect(ui->rbText, &QRadioButton::toggled,
            this, &TcpListener::onInputFormatChanged);

    updateCodecs();
    updateStatus();
}

/********************************************************/

TcpListener::~TcpListener()
{
    m_TcpServer.close();
    delete ui;
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

void TcpListener::doConnect()
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
    if (m_TcpServer.isListening() && m_Handler) {
        MessageHandlerWgt *editor = findChild<MessageHandlerWgt*>();
        if (editor) {
            m_Handler->setSettings(editor->settings());
        }
        m_Handler->doConnect(ui->rbBinary->isChecked());
        if (m_Handler->hasError()) {
            ui->textLog->append(QString("%1 : %2")
                                .arg(m_Handler->name(), m_Handler->lastError()));
        }
    }
    updateStatus();
}

/********************************************************/

void TcpListener::doDisconnect()
{
    m_TcpServer.close();
    if (m_Handler) {
        m_Handler->doDisconnect();
    }
    updateStatus();
}

/********************************************************/

void TcpListener::onInputFormatChanged()
{
    ui->cmbCodec->setVisible(ui->rbText->isChecked());
}

/********************************************************/

void TcpListener::changeReplyType(int index)
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

void TcpListener::changeHandler(int index)
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
    case ActionHandler::GateActionHandler:
        m_Handler = new GateHandler(this);
        editor = new GateHandlerWidget(this);
        break;
    }
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

/********************************************************/

void TcpListener::updateStatus()
{
    if (m_TcpServer.isListening()) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the TCP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("TCP [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("Choose TCP port to listen"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxAction->setEnabled(true);
        emit tabText(QString("TCP [-]"));
    }
}

/********************************************************/

void TcpListener::updateCodecs()
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, IODecoder::findCodecs()) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
}

/********************************************************/

QByteArray TcpListener::processData(const QHostAddress &host, const QByteArray &data)
{
    int mib = ui->cmbCodec->itemData(ui->cmbCodec->currentIndex()).toInt();
    ioDecoder.setMib(mib);
    QString displayData = ioDecoder.toUnicode(data, ui->rbBinary->isChecked());

    // log payload data
    ui->textLog->append(QString("%1 -> %2")
                        .arg(host.toString(), displayData));
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
                                .arg(host.toString(), m_Handler->lastError()));
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
