#include "udplistener.h"
#include "ui_udplistener.h"

#include <QNetworkDatagram>
#include <QTextCodec>

#include <QMessageBox>

#include <QDebug>

/********************************************************/

UdpListener::UdpListener(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UdpListener),
    m_UdpSocket(Q_NULLPTR)
{
    ui->setupUi(this);
    // configure UI default state
    ui->chkText->setChecked(true);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

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
        QByteArray replyData = processDatagram(datagram.senderAddress(), datagram.data());
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
    updateStatus();
}

/********************************************************/

void UdpListener::on_chkText_stateChanged(int arg1)
{
    ui->cmbCodec->setVisible(arg1 == Qt::CheckState::Checked);
}

/********************************************************/

void UdpListener::on_cmbReplyType_currentIndexChanged(int index)
{
    ui->editReply->setVisible(index == ReplyType::PredefinedReply);
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

QByteArray UdpListener::processDatagram(const QHostAddress &host, const QByteArray &data)
{
    QString displayData;
    if (ui->chkText->isChecked()) {
        int mib = ui->cmbCodec->itemData(ui->cmbCodec->currentIndex()).toInt();
        const QTextCodec *codec = QTextCodec::codecForMib(mib);
        const QString name = QLatin1String(codec->name());
        QTextCodec::ConverterState state;
        displayData = codec->toUnicode(data.constData(), data.size(), &state);
    } else {
        displayData = QString::fromStdString(data.toHex().toStdString());
    }

    // log payload data
    ui->textLog->append(QString("%1 -> %2")
                        .arg(host.toString())
                        .arg(displayData));
    ui->textLog->moveCursor(QTextCursor::End);
    // make reply
    switch (ui->cmbReplyType->currentIndex()) {
    case ReplyType::NoReply:
        break;
    case ReplyType::EchoReply:
        return data;
    case ReplyType::PredefinedReply:
        return ui->editReply->text().toUtf8();
    }
    return QByteArray();
}

/********************************************************/
