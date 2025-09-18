#include "socketlistener.h"
#include "ui_socketlistener.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QTextCodec>
#include <QMessageBox>

#include <QDebug>

/********************************************************/

SocketListener::SocketListener(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::SocketListener),
    m_LocalServer(this)
{
    ui->setupUi(this);
    ui->cmbDecoder->addItems(decoders());
    ui->cmbHandler->addItems(handlers());

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &SocketListener::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &SocketListener::doDisconnect);
    connect(ui->cmbReplyType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SocketListener::changeReplyType);
    connect(ui->cmbDecoder, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SocketListener::changeDecoder);
    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SocketListener::changeHandler);

    setupUiDefaultState();

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
    disconnect(this, nullptr, nullptr, nullptr);
    doDisconnect();
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

    printInfo(QString::number(clientSocket->socketDescriptor()),
              "connected to server !");
}

/********************************************************/

void SocketListener::onLocalSocketStateChanged(QLocalSocket::LocalSocketState  socketState)
{
    if (socketState == QLocalSocket::UnconnectedState) {
        auto clientSocket = qobject_cast<QLocalSocket*>(QObject::sender());
        if (!clientSocket) return;
        printInfo(QString::number(clientSocket->socketDescriptor()),
                  "disconnected from server !");
        clientSocket->deleteLater();
    }
}

/********************************************************/

void SocketListener::onReadyRead()
{
    QLocalSocket* sender = qobject_cast<QLocalSocket*>(QObject::sender());
    if (!sender) return;
    QByteArray data = sender->readAll();
    QByteArray replyData = processData(sender->socketDescriptor(), data);
    if (!replyData.isEmpty()) {
        sender->write(replyData);
    }
}

/********************************************************/

QTextBrowser *SocketListener::textLog() const
{
    return ui->textLog;
}

/********************************************************/

void SocketListener::doConnect()
{
    QString socketName = ui->editSocket->text();
    if (m_LocalServer.listen(socketName)) {
        connect(&m_LocalServer, &QLocalServer::newConnection,
                this, &SocketListener::onNewConnection);
    } else {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Socket %1 connection error!\n%2")
                              .arg(socketName, m_LocalServer.errorString()));
    }
    if (m_LocalServer.isListening()) {
        if (initDecoder(ui->rbBinary->isChecked())) {
            connect(decoder(), &MessageHandler::logMessage,
                    this, &SocketListener::printMessage);
            connect(decoder(), &MessageHandler::logError,
                    this, &SocketListener::printError);
        }
        if (initHandler(ui->rbBinary->isChecked())) {
            connect(handler(), &MessageHandler::logMessage,
                    this, &SocketListener::printMessage);
            connect(handler(), &MessageHandler::logError,
                    this, &SocketListener::printError);
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

void SocketListener::doDisconnect()
{
    m_LocalServer.close();
    disconnectDecoder();
    disconnectHandler();
    updateStatus();
}

/********************************************************/

void SocketListener::onInputFormatChanged()
{
    ui->cmbCodec->setVisible(ui->rbText->isChecked());
}

/********************************************************/

void SocketListener::changeDecoder(int index)
{
    auto editor = updateDecoder(index);
    if (editor) {
        ui->boxDecoder->layout()->addWidget(editor);
    }
}

/********************************************************/

void SocketListener::changeReplyType(int index)
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

void SocketListener::changeHandler(int index)
{
    auto editor = updateHandler(index);
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

/********************************************************/

void SocketListener::setupUiDefaultState()
{
    ui->rbBinary->setChecked(false);
    ui->rbText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

/********************************************************/

void SocketListener::updateStatus()
{
    if (m_LocalServer.isListening()) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the Socket</font>"));
        ui->editSocket->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxDecoder->setEnabled(false);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("Socket [%1]").arg(ui->editSocket->text()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">Socket to listen</font>"));
        ui->editSocket->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxDecoder->setEnabled(true);
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
    IODecoder ioDecoder(mib);
    QString textData    = ioDecoder.toUnicode(data, ui->rbBinary->isChecked());
    QString displayHost = QString::number(socketDescriptor);

    // log payload data
    printMessage(QString::number(socketDescriptor), textData);

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
