#include "dtsconnect.h"
#include "ui_dtsconnect.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QNetworkDatagram>
#include <QTextCodec>
#include <QMessageBox>

#include <QDebug>

#define ERROR_RET -1
#define OK_RET 0
#define WARNING_RET 1

/********************************************************/

DtsConnect::DtsConnect(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::DtsConnect)
{
    ui->setupUi(this);

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &DtsConnect::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &DtsConnect::doDisconnect);
    connect(this, &DtsConnect::dataReceived,
            this, &DtsConnect::doReceive);

    setupUiDefaultState();

    updateStatus();
}

/********************************************************/

DtsConnect::~DtsConnect()
{
    disconnect(this, nullptr, nullptr, nullptr);
    doDisconnect();
    delete ui;
}

/********************************************************/

QTextBrowser *DtsConnect::textLog() const
{
    return ui->textLog;
}

/********************************************************/

void DtsConnect::doConnect()
{
    const quint16 port = ui->spinPort->value();

    d.dtsClient.reset( new DTSClient(DtsConnect::rcvFunc, this, Q_NULLPTR, Q_NULLPTR, port));
//    d.dtsClient.reset( new DTSClient(Q_NULLPTR, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR, port));

    if( d.dtsClient->getState() != Ok ) {
        printError("DTS Client error", QString("(%1) %2").arg(d.dtsClient->getError()).arg(d.dtsClient->getErrStr()));
        d.dtsClient.reset();
    }

//    d.timer.setSingleShot(true);
//    connect(&d.timer, &QTimer::timeout,
//            this, &DtsConnect::doReadData);
//    d.timer.setInterval(1000);
//    d.timer.start();
    updateStatus();
}

/********************************************************/

void DtsConnect::doDisconnect()
{
    d.timer.stop();
    if (!d.dtsClient.isNull()) {
        d.dtsClient->stopSend();
        d.dtsClient->stopReceive();
        d.dtsClient.reset();
    }
    disconnectDecoder();
    disconnectHandler();
    updateStatus();
}

/********************************************************/

void DtsConnect::doReceive(DtsValue v, int32_t chnlId)
{
    QMutexLocker locker(&(d.mutex));
    // Обработка сигналов
    switch( v.type ) {
    case Ana_VT :
        printMessage(QString("Receive CH[%1]").arg(chnlId),
                     QString("ANA[%1]:%2(%3)").arg(v.idx).arg(v.value.toFloat(), 0, 'f', 5).arg(v.quality));
        break;
    case Bin_VT :
        printMessage(QString("Receive CH[%1]").arg(chnlId),
                     QString("BIN[%1]:%2(%3)").arg(v.idx).arg(v.value.toInt()).arg(v.quality));
        break;
    case Int_VT :
        printMessage(QString("Receive CH[%1]").arg(chnlId),
                     QString("INT[%1]:%2(%3)").arg(v.idx).arg(v.value.toInt()).arg(v.quality));
        break;
    case Grp_VT :
        printMessage(QString("Receive CH[%1]").arg(chnlId),
                     QString("GRP[%1]:%2(%3)").arg(v.idx).arg(QString::fromLatin1(v.value.toByteArray().toHex())).arg(v.quality));
        break;
    default :
        break;
    }
}

/********************************************************/

void DtsConnect::setupUiDefaultState()
{
    ui->rbBinary->setChecked(true);
    ui->rbText->setChecked(false);
    ui->cmbReplyType->setCurrentIndex(ReplyType::NoReply);
    ui->editReply->setHidden(true);

    ui->boxDecoder->setEnabled(false);
    ui->boxAction->setEnabled(false);
    ui->boxInput->setEnabled(false);
    ui->boxReply->setEnabled(false);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

/********************************************************/

void DtsConnect::updateStatus()
{
    if (!d.dtsClient.isNull()) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Connected to the DTS</font>"));
        ui->editHost->setEnabled(false);
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        emit tabText(QString("DTS client [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">Choose DTS server to connect</font>"));
        ui->editHost->setEnabled(true);
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        emit tabText(QString("DTS client [-]"));
    }
}

/********************************************************/

int DtsConnect::rcvFunc( void * argPtr, Value & value, int32_t chnlId )
{
    if (argPtr == Q_NULLPTR) return -1;

    auto *objPtr = static_cast<QObject *>(argPtr);
    auto *dtsPtr = qobject_cast<DtsConnect *>(objPtr);

    if (dtsPtr == Q_NULLPTR) return -1;

    DtsValue dtsValue(value);

    dtsPtr->doReceive(dtsValue, chnlId);
//    emit dtsPtr->dataReceived(dtsValue, chnlId);

    return 0;
}

DtsConnect::DtsValue::DtsValue(Value &v)
{
    this->type = v.type;
    this->idx  = v.idx;
    this->dt   = QDateTime::fromSecsSinceEpoch(v.sec).addMSecs(v.msec);

    switch( v.type ) {
    case Ana_VT :
    {
        AData * dataPtr = Q_NULLPTR;
        v.getData( dataPtr );
        this->value = dataPtr->value;
    }
        break;
    case Bin_VT :
    {
        BData * dataPtr = Q_NULLPTR;
        v.getData( dataPtr );
        this->value = dataPtr->value;
    }
        break;
    case Int_VT :
    {
        IData * dataPtr = Q_NULLPTR;
        v.getData( dataPtr );
        this->value = dataPtr->value;
    }
        break;
    case Grp_VT :
    {
        GData * dataPtr = NULL;
        v.getData( dataPtr );
        this->value = QByteArray(reinterpret_cast<const char *>(dataPtr->value), dataPtr->size);
    }
        break;
    default : break;
    }
}
