#include "dtsconnect.h"
#include "ui_dtsconnect.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QNetworkDatagram>
#include <QTextCodec>
#include <QMessageBox>

#include <QDebug>

/*
 * Предупреждение:
 * В режиме отладки при вызове функции DtsAgent::doDisconnect() возникает сообщение об ошибке и остановке приложения Real-time event 32 (SIG32).
 * В данном случае SIG32 — это не ошибка, а ожидаемое поведение.
 * Судя по всему, отладчик перехватывает внутренние события библиотеки потоков (pthreads).
 * Библиотека dts явно работает с потоками (создание, синхронизация), поэтому нужно указать отладчику игнорировать этот сигнал.
 *
 * Необходимо добавить в файл ~/.gdbinit строку:
 *
 * handle SIG32 nostop noprint
 */

enum {
    ERROR_RET   = -1,
    OK_RET      = 0,
    WARNING_RET = 1
};

DtsValue::DtsValue(Value &v)
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

DtsAgent::DtsAgent(QObject *parent) : QObject(parent)
{
    dtsClient = Q_NULLPTR;
}

DtsAgent::~DtsAgent()
{
    doDisconnect();
}

void DtsAgent::receiveData(Value &value, int32_t chnlId)
{
    DtsValue dtsValue(value);
    emit dataReceived(dtsValue, chnlId);
}

bool DtsAgent::doConnect(quint16 port, QString *err)
{
    if (dtsClient) {
        return true;
    }
    dtsClient = new DTSClient(DtsAgent::rcvFunc, this, Q_NULLPTR, Q_NULLPTR, port);
    if (dtsClient->getState() != Ok ) {
        if (err) err->append(QString("(%1) %2").arg(dtsClient->getError()).arg(dtsClient->getErrStr()));
        delete dtsClient;
        dtsClient = Q_NULLPTR;
        return false;
    }
    return true;
}

void DtsAgent::doDisconnect()
{
    if (dtsClient) {
        delete dtsClient;
        dtsClient = Q_NULLPTR;
    }
}

int DtsAgent::rcvFunc( void * argPtr, Value & value, int32_t chnlId )
{
    if (argPtr == Q_NULLPTR) {
        qWarning() << "DtsAgent: arg pointer is Q_NULLPTR";
        return -1;
    }

    auto *objPtr = static_cast<QObject *>(argPtr);
    auto *dtsPtr = qobject_cast<DtsAgent *>(objPtr);

    if (dtsPtr == Q_NULLPTR) {
        qWarning() << "DtsAgent: DTS pointer is Q_NULLPTR";
        return -1;
    }

    dtsPtr->receiveData(value, chnlId);

    return 0;
}


/********************************************************/

DtsConnect::DtsConnect(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::DtsConnect)
{
    ui->setupUi(this);

    d.dtsAgent.reset(new DtsAgent(this));

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &DtsConnect::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &DtsConnect::doDisconnect);
    connect(d.dtsAgent.data(), &DtsAgent::dataReceived,
            this, &DtsConnect::doReceive, Qt::QueuedConnection);

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
    QString err;
    const quint16 port = ui->spinPort->value();
    if (!d.dtsAgent->doConnect(port, &err)) {
        printError("DTS Client error", err);
    }
    updateStatus();
}

/********************************************************/

void DtsConnect::doDisconnect()
{
    d.timer.stop();
    d.dtsAgent->doDisconnect();
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
    if (d.dtsAgent->isActive()) {
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

