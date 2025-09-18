#include "udplistener.h"
#include "ui_udplistener.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QNetworkDatagram>
#include <QTextCodec>
#include <QMessageBox>

#include <QDebug>

/********************************************************/

UdpListener::UdpListener(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::UdpListener),
    m_UdpSocket(Q_NULLPTR)
{
    ui->setupUi(this);
    ui->cmbDecoder->addItems(decoders());
    ui->cmbHandler->addItems(handlers());

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &UdpListener::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &UdpListener::doDisconnect);
    connect(ui->cmbReplyType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &UdpListener::changeReplyType);
    connect(ui->cmbDecoder, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &UdpListener::changeDecoder);
    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &UdpListener::changeHandler);

    setupUiDefaultState();

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
    disconnect(this, nullptr, nullptr, nullptr);
    doDisconnect();
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

QTextBrowser *UdpListener::textLog() const
{
    return ui->textLog;
}

/********************************************************/

void UdpListener::doConnect()
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
    if (m_UdpSocket) {
        if (initDecoder(ui->rbBinary->isChecked())) {
            connect(decoder(), &MessageHandler::logMessage,
                    this, &UdpListener::printMessage);
            connect(decoder(), &MessageHandler::logError,
                    this, &UdpListener::printError);
        }
        if (initHandler(ui->rbBinary->isChecked())) {
            connect(handler(), &MessageHandler::logMessage,
                    this, &UdpListener::printMessage);
            connect(handler(), &MessageHandler::logError,
                    this, &UdpListener::printError);
        }
    }
    const auto decodeErrors = decoderErrors();
    for (const auto &error : decodeErrors) {
        printError(decoderName(), error);
    }
    const auto actionErrors = handlerErrors();
    for (const auto &error : actionErrors) {
        printError(handlerName(), error);
    }
    updateStatus();
}

/********************************************************/

void UdpListener::doDisconnect()
{
    if (m_UdpSocket) {
        m_UdpSocket->close();
        m_UdpSocket->deleteLater();
        m_UdpSocket = Q_NULLPTR;
    }
    disconnectDecoder();
    disconnectHandler();
    updateStatus();
}

/********************************************************/

void UdpListener::onInputFormatChanged()
{
    ui->cmbCodec->setVisible(ui->rbText->isChecked());
}

/********************************************************/

void UdpListener::changeDecoder(int index)
{
    auto editor = updateDecoder(index);
    if (editor) {
        ui->boxDecoder->layout()->addWidget(editor);
    }
}

/********************************************************/

void UdpListener::changeReplyType(int index)
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

void UdpListener::changeHandler(int index)
{
    auto editor = updateHandler(index);
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

/********************************************************/

void UdpListener::setupUiDefaultState()
{
    ui->rbBinary->setChecked(false);
    ui->rbText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

/********************************************************/

void UdpListener::updateStatus()
{
    if (m_UdpSocket) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the UDP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxDecoder->setEnabled(false);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("UDP [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">Choose UDP port to listen</font>"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxDecoder->setEnabled(true);
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
    IODecoder ioDecoder(mib);
    QString textData    = ioDecoder.toUnicode(data, ui->rbBinary->isChecked());
    QString displayHost = host.toString();

    // log payload data
    printMessage(displayHost, textData);

    // Decoder
    QByteArray binData;
    if (ui->rbText->isChecked()) {
        binData = doDecode(textData);
    } else {
        binData = doDecode(data);
    }
    // log decode errors
    const auto decodeErrors = decoderErrors();
    for (const auto &error : decodeErrors) {
        printError(displayHost, error);
    }

    // Handler
    QByteArray reply;
    if (ui->rbText->isChecked()) {
        reply = doHandle(QString(binData));
    } else {
        reply = doHandle(binData);
    }
    // log errors
    const auto actionErrors = handlerErrors();
    for (const auto &error : actionErrors) {
        printError(displayHost, error);
    }

    // log reply data
    if (!reply.isEmpty()) {
        QString replyData = ioDecoder.toUnicode(reply, ui->rbBinary->isChecked());
        printReply(host.toString(), replyData);
    }
    clearErrors();

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
    case ReplyType::ActionReply:
        break;
    }
    return reply;
}

/********************************************************/
