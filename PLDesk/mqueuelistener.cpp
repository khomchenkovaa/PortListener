#include "mqueuelistener.h"
#include "ui_mqueuelistener.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QTextCodec>
#include <QMessageBox>
#include <QTimer>

enum {
    MQ_MSG_SIZE = 8192
};

MQueueListener::MQueueListener(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::MQueueListener)
{
    ui->setupUi(this);
    ui->cmbDecoder->addItems(decoders());
    ui->cmbHandler->addItems(handlers());

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &MQueueListener::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &MQueueListener::doDisconnect);
    connect(ui->cmbReplyType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MQueueListener::changeReplyType);
    connect(ui->cmbDecoder, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MQueueListener::changeDecoder);
    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MQueueListener::changeHandler);

    setupUiDefaultState();

    connect(ui->rbBinary, &QRadioButton::toggled,
            this, &MQueueListener::onInputFormatChanged);
    connect(ui->rbText, &QRadioButton::toggled,
            this, &MQueueListener::onInputFormatChanged);

    mq_worker = new QMQueue(this);
    connect(mq_worker, &QMQueue::connected, this, [this](){
        printInfo(mq_worker->queueName(), "Connected");
    });
    connect(mq_worker, &QMQueue::disconnected, this, [this](){
        printInfo(mq_worker->queueName(), "Disonnected");
    });
    connect(mq_worker, &QMQueue::errorOccurred, this, [this](QMQueue::MQueueError queueError){
        printError(mq_worker->queueName(), QMQueue::mqErrorString(queueError));
    });
    connect(mq_worker, &QMQueue::stateChanged,  this, [this](QMQueue::MQueueState socketState){
        printMessage(mq_worker->queueName(), QMQueue::mqStateString(socketState));
    });
    connect(mq_worker, &QIODevice::readyRead,   this, &MQueueListener::onReadyRead);

    updateStatus();
    updateCodecs();
}

MQueueListener::~MQueueListener()
{
    if (mq_worker->isOpen()) doDisconnect();
    disconnect(mq_worker, nullptr, nullptr, nullptr);
    mq_worker->deleteLater();
    disconnect(this, nullptr, nullptr, nullptr);
    delete ui;
}

QTextBrowser *MQueueListener::textLog() const
{
    return ui->textLog;
}

void MQueueListener::onReadyRead()
{
    while(mq_worker->messages()) {
        QByteArray data = mq_worker->read(MQ_MSG_SIZE);
        if (data.size()) {
            QByteArray replyData = processData(mq_worker->queueName(), data);
            if (!replyData.isEmpty()) {
                // TODO write data to reply mqueue
                //sender->write(replyData);
            }
        } else {
            printError("MQ", QString("Cannot read message from queue: %1").arg(mq_worker->errorString()));
            break;
        }
    }
}

void MQueueListener::doConnect()
{
    QString mqName = ui->editConnection->text();
    mq_worker->setInterval(ui->spinInterval->value());
    if (!mq_worker->connectToQueue(mqName)) {
        printInfo("MQ", mq_worker->errorString());
        return;
    }
    printInfo("MQ", QString("Data exchange started, interval %1 ms.").arg(ui->spinInterval->value()));

    if (initDecoder(ui->rbBinary->isChecked())) {
        connect(decoder(), &MessageHandler::logMessage,
                this, &MQueueListener::printMessage);
        connect(decoder(), &MessageHandler::logError,
                this, &MQueueListener::printError);
    }
    if (initHandler(ui->rbBinary->isChecked())) {
        connect(handler(), &MessageHandler::logMessage,
                this, &MQueueListener::printMessage);
        connect(handler(), &MessageHandler::logError,
                this, &MQueueListener::printError);
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

void MQueueListener::doDisconnect()
{
    mq_worker->disconnectFromQueue();
    printInfo("MQ", "Data exchanging stopped.");
    disconnectDecoder();
    disconnectHandler();
    updateStatus();
}

void MQueueListener::onInputFormatChanged()
{
    ui->cmbCodec->setVisible(ui->rbText->isChecked());
}

void MQueueListener::changeDecoder(int index)
{
    auto editor = updateDecoder(index);
    if (editor) {
        ui->boxDecoder->layout()->addWidget(editor);
    }
}

void MQueueListener::changeReplyType(int index)
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

void MQueueListener::changeHandler(int index)
{
    auto editor = updateHandler(index);
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

void MQueueListener::setupUiDefaultState()
{
    ui->rbBinary->setChecked(false);
    ui->rbText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

void MQueueListener::updateStatus()
{
    if (mq_worker->isOpen()) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the MQueue</font>"));
        ui->editConnection->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxDecoder->setEnabled(false);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("MQ [%1]").arg(ui->editConnection->text()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">Queue to listen</font>"));
        ui->editConnection->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxDecoder->setEnabled(true);
        ui->boxAction->setEnabled(true);
        emit tabText(QString("MQ [-]"));
    }
}

void MQueueListener::updateCodecs()
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, IODecoder::findCodecs()) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
}

QByteArray MQueueListener::processData(const QString &mqName, const QByteArray &data)
{
    int mib = ui->cmbCodec->itemData(ui->cmbCodec->currentIndex()).toInt();
    IODecoder ioDecoder(mib);
    QString textData    = ioDecoder.toUnicode(data, ui->rbBinary->isChecked());
    QString displayHost = mqName;

    // log payload data
    printMessage(mqName, textData);

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
        printReply(mqName, replyData);
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
