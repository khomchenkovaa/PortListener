#include "udplistener.h"
#include "ui_udplistener.h"

#include "messagehandlerwgt.h"
#include "filehandler.h"

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

void UdpListener::setCodecList(const QList<QTextCodec *> &list)
{
    ui->cmbCodec->clear();
    foreach (const QTextCodec *codec, list) {
        ui->cmbCodec->addItem(QLatin1String(codec->name()),
                              QVariant(codec->mibEnum()));
    }
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
    if (m_Handler) {
        MessageHandlerWgt *editor = findChild<MessageHandlerWgt*>();
        if (editor) {
            m_Handler->setSettings(editor->settings());
        }
        m_Handler->connect(ui->rbBinary->isChecked());
        if (m_Handler->hasError()) {
            ui->textLog->append(QString("%1 : %2")
                                .arg(m_Handler->name())
                                .arg(m_Handler->lastError()));
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
        m_Handler->disconnect();
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
    }
    switch (index) {
    case ActionHandler::NoActionHandler:
        m_Handler = Q_NULLPTR;
        break;
    case ActionHandler::FileActionHandler:
        m_Handler = new FileHandler(this);
        break;
    }
    if (m_Handler) {
        editor = m_Handler->settingsWidget(this);
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
        emit tabText(QString("UDP [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("Choose UDP port to listen"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        emit tabText(QString("UDP [-]"));
    }
}

/********************************************************/

QByteArray UdpListener::processData(const QHostAddress &host, const QByteArray &data)
{
    int mib = ui->cmbCodec->itemData(ui->cmbCodec->currentIndex()).toInt();
    const QTextCodec *codec = QTextCodec::codecForMib(mib);
    const QString name = QLatin1String(codec->name());
    QTextCodec::ConverterState state;

    QString displayData;
    if (ui->rbText->isChecked()) {
        displayData = codec->toUnicode(data.constData(), data.size(), &state);
    } else {
        displayData = QString::fromLatin1(data.toHex());
    }

    QByteArray reply;
    // Handler
    if (m_Handler) {
        if (ui->rbText->isChecked()) {
            reply = m_Handler->processData(displayData);
        } else {
            reply = m_Handler->processData(data);
        }
    }

    // log payload data
    ui->textLog->append(QString("%1 -> %2")
                        .arg(host.toString())
                        .arg(displayData));
    ui->textLog->moveCursor(QTextCursor::End);

    // make reply
    switch (ui->cmbReplyType->currentIndex()) {
    case ReplyType::NoReply:
        reply.clear();
        break;
    case ReplyType::EchoReply:
        reply = data;
        break;
    case ReplyType::TextReply:
        reply = codec->fromUnicode(ui->editReply->text());
        break;
    case ReplyType::BinaryReply:
        reply = QByteArray::fromHex(ui->editReply->text().toLatin1());
        break;
    }
    return reply;
}

/********************************************************/
