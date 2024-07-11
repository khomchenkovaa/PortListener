#include "udplistener.h"
#include "ui_udplistener.h"

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

#include <QNetworkDatagram>
#include <QTextCodec>

#include <QMessageBox>

#include <QDebug>

/********************************************************/

UdpListener::UdpListener(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UdpListener),
    m_UdpSocket(Q_NULLPTR),
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
            this, &UdpListener::onInputFormatChanged);
    connect(ui->rbText, &QRadioButton::toggled,
            this, &UdpListener::onInputFormatChanged);

    updateCodecs();
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
        QByteArray replyData = processData(datagram.senderAddress(), datagram.data());
        if (!replyData.isEmpty()) {
            m_UdpSocket->writeDatagram(datagram.makeReply(replyData));
        }
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
    if (m_UdpSocket && m_Handler) {
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

void UdpListener::on_btnDisconnect_clicked()
{
    if (m_UdpSocket) {
        m_UdpSocket->close();
        m_UdpSocket->deleteLater();
        m_UdpSocket = Q_NULLPTR;
    }
    if (m_Handler) {
        m_Handler->doDisconnect();
    }
    updateStatus();
}

/********************************************************/

void UdpListener::onInputFormatChanged()
{
    ui->cmbCodec->setVisible(ui->rbText->isChecked());
}

/********************************************************/

void UdpListener::on_cmbReplyType_currentIndexChanged(int index)
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

void UdpListener::on_cmbHandler_currentIndexChanged(int index)
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

void UdpListener::updateStatus()
{
    if (m_UdpSocket) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the UDP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("UDP [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("Choose UDP port to listen"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxAction->setEnabled(true);
        emit tabText(QString("UDP [-]"));
    }
}

/********************************************************/

void UdpListener::updateCodecs()
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, IODecoder::findCodecs()) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
}

/********************************************************/

QByteArray UdpListener::processData(const QHostAddress &host, const QByteArray &data)
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
            ui->textLog->append(QString("%1 -> %2").arg(host.toString(), m_Handler->lastError()));
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
