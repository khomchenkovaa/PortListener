#include "datasender.h"
#include "ui_datasender.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QTextCodec>

/********************************************************/

DataSender::DataSender(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::DataSender)
{
    ui->setupUi(this);
    ui->cmbHandler->addItems(handlers());

    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataSender::changeHandler);

    setupUiDefaultState();

    connect(ui->rbBinary, &QRadioButton::toggled,
            this, &DataSender::onInputFormatChanged);
    connect(ui->rbText, &QRadioButton::toggled,
            this, &DataSender::onInputFormatChanged);
    connect(ui->btnSend, &QAbstractButton::clicked,
            this, &DataSender::doSend);

    updateCodecs();
}

/********************************************************/

DataSender::~DataSender()
{
    delete ui;
}

/********************************************************/

QTextBrowser *DataSender::textLog() const
{
    return ui->textLog;
}

/********************************************************/

void DataSender::doSend()
{
    // do connnect if not connected
    if (!isHandlerConnected()) doConnect();

    // process data
    bool binary = ui->rbBinary->isChecked();
    int mib = ui->cmbCodec->itemData(ui->cmbCodec->currentIndex()).toInt();
    IODecoder ioDecoder(mib);

    QString text = ui->editData->text();
    printMessage(handlerName(), text);

    QByteArray reply;
    if (binary) {
        QByteArray data = ioDecoder.fromUnicode(text, binary);
        reply = doHandle(data);
    } else {
        reply = doHandle(text);
    }
    const auto sendErrors = handlerErrors();
    for (const auto &error : sendErrors) {
        printError(handlerName(), error);
    }

    // log reply data
    if (!reply.isEmpty()) {
        QString replyData = ioDecoder.toUnicode(reply, binary);
        printReply(handlerName(), replyData);
    }

    // do not disconnect
}

/********************************************************/

void DataSender::doConnect()
{
    if (initHandler(ui->rbBinary->isChecked())) {
        connect(handler(), &MessageHandler::logMessage,
                this, &DataSender::printMessage);
        connect(handler(), &MessageHandler::logError,
                this, &DataSender::printError);
    }
    const auto connErrors = handlerErrors();
    for (const auto &error : connErrors) {
        printError(handlerName(), error);
    }
}

/********************************************************/

void DataSender::onInputFormatChanged()
{
    ui->cmbCodec->setVisible(ui->rbText->isChecked());
}

/********************************************************/

void DataSender::changeHandler(int index)
{
    auto editor = updateHandler(index);
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

/********************************************************/

void DataSender::setupUiDefaultState()
{
    ui->rbBinary->setChecked(false);
    ui->rbText->setChecked(true);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

/********************************************************/

void DataSender::updateCodecs()
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, IODecoder::findCodecs()) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
}

/********************************************************/
