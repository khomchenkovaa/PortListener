#include "tcplistener.h"
#include "ui_tcplistener.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QHostAddress>
#include <QTcpSocket>
#include <QTextCodec>
#include <QMessageBox>

#include <QDebug>

/********************************************************/

TcpListener::TcpListener(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::TcpListener),
    m_TcpServer(this)
{
    ui->setupUi(this);
    ui->cmbDecoder->addItems(decoders());
    ui->cmbHandler->addItems(handlers());

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &TcpListener::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &TcpListener::doDisconnect);
    connect(ui->cmbReplyType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TcpListener::changeReplyType);
    connect(ui->cmbDecoder, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TcpListener::changeDecoder);
    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TcpListener::changeHandler);

    setupUiDefaultState();

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
    if (m_TcpServer.isListening()) doDisconnect();
    disconnect(&m_TcpServer, nullptr, nullptr, nullptr);
    disconnect(this, nullptr, nullptr, nullptr);
    delete ui;
}

/********************************************************/

void TcpListener::onNewConnection()
{
    auto clientSocket = m_TcpServer.nextPendingConnection();
    if (!clientSocket) return;
    connect(clientSocket, &QAbstractSocket::readyRead,
           this, &TcpListener::onReadyRead);
    connect(clientSocket, &QAbstractSocket::stateChanged,
           this, &TcpListener::onTcpSocketStateChanged);
    connect(this, &TcpListener::closeAll,
            clientSocket, &QAbstractSocket::disconnectFromHost);

    printInfo(clientSocket->peerAddress().toString(), "connected to server !");
}

/********************************************************/

void TcpListener::onTcpSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState) {
        QTcpSocket* clientSocket = static_cast<QTcpSocket*>(QObject::sender());
        printInfo(clientSocket->peerAddress().toString(), "disconnected from server !");
        clientSocket->deleteLater();
    }
}

/********************************************************/

void TcpListener::onReadyRead()
{
    auto sender = static_cast<QTcpSocket*>(QObject::sender());
    auto data = sender->readAll();
    auto replyData = processData(sender->peerAddress(), data);
    if (!replyData.isEmpty()) {
        sender->write(replyData);
        if (!sender->waitForBytesWritten(1000)) {
            printError(sender->peerAddress().toString(), "Problems with send reply");
        }
    }
}

/********************************************************/

QTextBrowser *TcpListener::textLog() const
{
    return ui->textLog;
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
    if (m_TcpServer.isListening()) {
        if (initDecoder(ui->rbBinary->isChecked())) {
            connect(decoder(), &MessageHandler::logMessage,
                    this, &TcpListener::printMessage);
            connect(decoder(), &MessageHandler::logError,
                    this, &TcpListener::printError);
        }
        if (initHandler(ui->rbBinary->isChecked())) {
            connect(handler(), &MessageHandler::logMessage,
                    this, &TcpListener::printMessage);
            connect(handler(), &MessageHandler::logError,
                    this, &TcpListener::printError);
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

void TcpListener::doDisconnect()
{
    m_TcpServer.close();
    emit closeAll();
    disconnectDecoder();
    disconnectHandler();
    updateStatus();
}

/********************************************************/

void TcpListener::onInputFormatChanged()
{
    ui->cmbCodec->setVisible(ui->rbText->isChecked());
}

/********************************************************/

void TcpListener::changeDecoder(int index)
{
    auto editor = updateDecoder(index);
    if (editor) {
        ui->boxDecoder->layout()->addWidget(editor);
    }
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
    auto editor = updateHandler(index);
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

/********************************************************/

void TcpListener::setupUiDefaultState()
{
    ui->rbBinary->setChecked(false);
    ui->rbText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

/********************************************************/

void TcpListener::updateStatus()
{
    if (m_TcpServer.isListening()) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the TCP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxDecoder->setEnabled(false);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("TCP [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">Choose TCP port to listen</font>"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxDecoder->setEnabled(true);
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
        printReply(displayHost, replyData);
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
