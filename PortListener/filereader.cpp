#include "filereader.h"
#include "ui_filereader.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QTextCodec>
#include <QMessageBox>
#include <QFileDialog>

FileReader::FileReader(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::FileReader)
{
    ui->setupUi(this);
    ui->cmbDecoder->addItems(decoders());
    ui->cmbHandler->addItems(handlers());

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &FileReader::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &FileReader::doDisconnect);
    connect(ui->cmbReplyType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FileReader::changeReplyType);
    connect(ui->cmbDecoder, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FileReader::changeDecoder);
    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FileReader::changeHandler);
    connect(ui->btnConnection, &QAbstractButton::clicked, this, [this](){
        QString fileName = QFileDialog::getOpenFileName(this, "Open file to read", QFileInfo(ui->editConnection->text()).absoluteFilePath());
        if (!fileName.isEmpty()) ui->editConnection->setText(fileName);
    });

    setupUiDefaultState();

    connect(ui->rbBinary, &QRadioButton::toggled,
            this, &FileReader::onInputFormatChanged);
    connect(ui->rbText, &QRadioButton::toggled,
            this, &FileReader::onInputFormatChanged);
    connect(ui->rbHex, &QRadioButton::toggled,
            this, &FileReader::onInputFormatChanged);
    connect(ui->cmbReadMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FileReader::onInputFormatChanged);

    updateStatus();
    updateCodecs();
}

FileReader::~FileReader()
{
    if (d.file.isOpen()) doDisconnect();
    disconnect(this, nullptr, nullptr, nullptr);
    delete ui;
}

QTextBrowser *FileReader::textLog() const
{
    return ui->textLog;
}

void FileReader::onReadyRead()
{
    if (d.file.atEnd() && d.rotate) d.file.seek(0);
    QByteArray data;
    if (d.readMode == ReadMode::READ_ALL) {
        data = d.file.readAll();
    } else {
        if (d.fileFormat == FileFormat::BIN_CONTENT) {
            data = d.file.read(d.binLength);
        } else {
            data = d.file.readLine();
        }
    }
    if (data.size()) {
        if (d.fileFormat == FileFormat::HEX_CONTENT) {
            processData(d.fileName(), QByteArray::fromHex(data));
        } else {
            processData(d.fileName(), data);
        }
    } else {
        printError(d.fileName(), "Cannot read data from file");
    }
}

void FileReader::doConnect()
{
    QString fileName = ui->editConnection->text();
    if (fileName.isEmpty()) {
        printError("FileReader", "File name is empty");
        return;
    }
    if (!QFileInfo::exists(fileName)) {
        printError("FileReader", QString("File '%1' doesn't exists").arg(QFileInfo(fileName).fileName()));
        return;
    }
    d.file.setFileName(fileName);
    if (ui->rbBinary->isChecked()) d.fileFormat = FileFormat::BIN_CONTENT;
    if (ui->rbText->isChecked())   d.fileFormat = FileFormat::PLAIN_TEXT;
    if (ui->rbHex->isChecked())    d.fileFormat = FileFormat::HEX_CONTENT;
    d.rotate = ui->chkRotate->isChecked();
    d.timer.setInterval(ui->spinInterval->value());
    d.readMode = static_cast<ReadMode>(ui->cmbReadMode->currentIndex());
    d.binLength = ui->spinBytes->value();

    QIODevice::OpenMode openMode = (d.fileFormat == FileFormat::BIN_CONTENT) ? (QIODevice::ReadOnly) : (QIODevice::ReadOnly | QIODevice::Text);
    if (!d.file.open(openMode)) {
        printError("FileReader", d.file.errorString());
        return;
    }    
    connect(&d.timer, &QTimer::timeout, this, &FileReader::onReadyRead);
    d.timer.start();
    printInfo("FileReader", QString("Data reading started, interval %1 ms.").arg(ui->spinInterval->value()));

    if (initDecoder(d.fileFormat == FileFormat::BIN_CONTENT)) {
        connect(decoder(), &MessageHandler::logMessage,
                this, &FileReader::printMessage);
        connect(decoder(), &MessageHandler::logError,
                this, &FileReader::printError);
    }
    if (initHandler(d.fileFormat == FileFormat::BIN_CONTENT)) {
        connect(handler(), &MessageHandler::logMessage,
                this, &FileReader::printMessage);
        connect(handler(), &MessageHandler::logError,
                this, &FileReader::printError);
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

void FileReader::doDisconnect()
{
    disconnect(&d.timer, nullptr, nullptr, nullptr);
    d.file.close();
    printInfo("FileReader", "Data reading stopped.");
    disconnectDecoder();
    disconnectHandler();
    updateStatus();
}

void FileReader::onInputFormatChanged()
{
    bool isBin  = ui->rbBinary->isChecked();
    bool isText = ui->rbText->isChecked();
    bool isHex  = ui->rbHex->isChecked();
    ui->cmbCodec->setVisible(isText);
    ui->cmbReadMode->setDisabled(isHex);
    ui->cmbReadMode->setItemText(ReadMode::READ_CHUNK, isBin ? "Read next bytes" : "Read next line");
    if (isHex) ui->cmbReadMode->setCurrentIndex(ReadMode::READ_CHUNK);
    ui->lblBytes->setVisible(isBin && ui->cmbReadMode->currentIndex());
    ui->spinBytes->setVisible(isBin && ui->cmbReadMode->currentIndex());
}

void FileReader::changeDecoder(int index)
{
    auto editor = updateDecoder(index);
    if (editor) {
        ui->boxDecoder->layout()->addWidget(editor);
    }
}

void FileReader::changeReplyType(int index)
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

void FileReader::changeHandler(int index)
{
    auto editor = updateHandler(index);
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

void FileReader::setupUiDefaultState()
{
    QStringList readModeItems = QStringList() << "Read All" << "Read next line";
    ui->cmbReadMode->addItems(readModeItems);
    ui->cmbReadMode->setCurrentIndex(ReadMode::READ_ALL);

    ui->lblBytes->setVisible(false);
    ui->spinBytes->setVisible(false);
    ui->spinBytes->setValue(d.binLength);

    ui->rbBinary->setChecked(false);
    ui->rbText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

void FileReader::updateStatus()
{
    if (d.file.isOpen()) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Reading the File</font>"));
        ui->editConnection->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxDecoder->setEnabled(false);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("File [%1]").arg(d.fileName()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">File to read</font>"));
        ui->editConnection->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxDecoder->setEnabled(true);
        ui->boxAction->setEnabled(true);
        emit tabText(QString("File [-]"));
    }
}

void FileReader::updateCodecs()
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, IODecoder::findCodecs()) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
}

QByteArray FileReader::processData(const QString &fileName, const QByteArray &data)
{
    int mib = ui->cmbCodec->itemData(ui->cmbCodec->currentIndex()).toInt();
    IODecoder ioDecoder(mib);
    QString textData    = ioDecoder.toUnicode(data, d.isBin());
    QString displayHost = fileName;

    // log payload data
    printMessage(fileName, textData);

    // Decoder
    QByteArray decodedData;
    if (d.isBin()) {
        decodedData = doDecode(data);
    } else {
        decodedData = doDecode(textData);
    }
    // log decode errors
    const auto decodeErrors = decoderErrors();
    for (const auto &error : decodeErrors) {
        printError(displayHost, error);
    }

    // Handler
    QByteArray reply;
    if (d.isBin()) {
        reply = doHandle(decodedData);
    } else {
        reply = doHandle(QString(decodedData));
    }
    // log errors
    const auto actionErrors = handlerErrors();
    for (const auto &error : actionErrors) {
        printError(displayHost, error);
    }

    // log reply data
    if (!reply.isEmpty()) {
        QString replyData = ioDecoder.toUnicode(reply, d.isBin());
        printReply(fileName, replyData);
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
