#include "tcpclient.h"
#include "ui_tcpclient.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QNetworkDatagram>
#include <QTextCodec>
#include <QMessageBox>

#include <QDebug>

/********************************************************/

TcpClient::TcpClient(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::TcpClient),
    m_TcpSocket(Q_NULLPTR)
{
    ui->setupUi(this);
    ui->cmbDecoder->addItems(decoders());
    ui->cmbHandler->addItems(handlers());

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &TcpClient::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &TcpClient::doDisconnect);
    connect(ui->cmbReplyType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TcpClient::changeReplyType);
    connect(ui->cmbDecoder, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TcpClient::changeDecoder);
    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TcpClient::changeHandler);

    setupUiDefaultState();

    connect(ui->rbBinary, &QRadioButton::toggled,
            this, &TcpClient::onInputFormatChanged);
    connect(ui->rbText, &QRadioButton::toggled,
            this, &TcpClient::onInputFormatChanged);

    updateCodecs();
    updateStatus();
}

/********************************************************/

TcpClient::~TcpClient()
{
    disconnect(this, nullptr, nullptr, nullptr);
    doDisconnect();
    delete ui;
}

/********************************************************/

void TcpClient::onReadyRead()
{
    QTcpSocket* sender = qobject_cast<QTcpSocket*>(QObject::sender());
    if (!sender) return;
    QByteArray data = sender->readAll();
    QByteArray replyData = processData(sender->peerAddress(), data);
    if (!replyData.isEmpty()) {
        sender->write(replyData);
        if (!sender->waitForBytesWritten(1000)) {
            printError(sender->peerAddress().toString(), "Problems with send reply");
        }
    }
}

/********************************************************/

QTextBrowser *TcpClient::textLog() const
{
    return ui->textLog;
}

/********************************************************/

void TcpClient::doConnect()
{
    const QString host = ui->editHost->text().trimmed();
    const quint16 port = ui->spinPort->value();
    m_TcpSocket = new QTcpSocket(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QObject::connect(m_TcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, [this](){
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Connect to TCP server %1:%2 error!\n%3")
                              .arg(host)
                              .arg(port)
                              .arg(m_TcpSocket->errorString()));
        });
#else
    QObject::connect(m_TcpSocket, &QAbstractSocket::errorOccurred, this, [this, host, port](){
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Connect to TCP server %1:%2 error!\n%3")
                              .arg(host)
                              .arg(port)
                              .arg(m_TcpSocket->errorString()));
    });
#endif

    m_TcpSocket->connectToHost(host, port);
    m_TcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    if (m_TcpSocket->waitForConnected()) {
        connect(m_TcpSocket, &QTcpSocket::readyRead,
               this, &TcpClient::onReadyRead);
    } else {
        m_TcpSocket->deleteLater();
        m_TcpSocket = Q_NULLPTR;
    }

    if (m_TcpSocket) {
        if (initDecoder(ui->rbBinary->isChecked())) {
            connect(decoder(), &MessageHandler::logMessage,
                    this, &TcpClient::printMessage);
            connect(decoder(), &MessageHandler::logError,
                    this, &TcpClient::printError);
        }
        if (initHandler(ui->rbBinary->isChecked())) {
            connect(handler(), &MessageHandler::logMessage,
                    this, &TcpClient::printMessage);
            connect(handler(), &MessageHandler::logError,
                    this, &TcpClient::printError);
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

void TcpClient::doDisconnect()
{
    if (m_TcpSocket) {
        m_TcpSocket->close();
        m_TcpSocket->deleteLater();
        m_TcpSocket = Q_NULLPTR;
    }
    disconnectDecoder();
    disconnectHandler();
    updateStatus();
}

/********************************************************/

void TcpClient::onInputFormatChanged()
{
    ui->cmbCodec->setVisible(ui->rbText->isChecked());
}

/********************************************************/

void TcpClient::changeDecoder(int index)
{
    auto editor = updateDecoder(index);
    if (editor) {
        ui->boxDecoder->layout()->addWidget(editor);
    }
}

/********************************************************/

void TcpClient::changeReplyType(int index)
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

void TcpClient::changeHandler(int index)
{
    auto editor = updateHandler(index);
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

/********************************************************/

void TcpClient::setupUiDefaultState()
{
    ui->rbBinary->setChecked(false);
    ui->rbText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

/********************************************************/

void TcpClient::updateStatus()
{
    if (m_TcpSocket) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Connected to the TCP server</font>"));
        ui->editHost->setEnabled(false);
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxDecoder->setEnabled(false);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("TCP client [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">Choose TCP server to connect</font>"));
        ui->editHost->setEnabled(true);
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxDecoder->setEnabled(true);
        ui->boxAction->setEnabled(true);
        emit tabText(QString("TCP client [-]"));
    }
}

/********************************************************/

void TcpClient::updateCodecs()
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, IODecoder::findCodecs()) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
}

/********************************************************/

QByteArray TcpClient::processData(const QHostAddress &host, const QByteArray &data)
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
