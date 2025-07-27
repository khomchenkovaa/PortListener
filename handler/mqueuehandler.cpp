#include "mqueuehandler.h"

MqueueHandler::MqueueHandler(QObject *parent)
    : MessageHandler(tr("Mqueue handler"), parent)
    , mq_worker(Q_NULLPTR)
{
}

MqueueHandler::~MqueueHandler()
{
    MqueueHandler::doDisconnect();
}

QByteArray MqueueHandler::processData(const QByteArray &data)
{
    mq_worker->write(data);
    return QByteArray();
}

QByteArray MqueueHandler::processData(const QString &data)
{
    mq_worker->write(data.toUtf8());
    return QByteArray();
}

void MqueueHandler::doConnect(bool binary)
{
    Q_UNUSED(binary)

    clearErrors();
    const auto mq = settings()->value(Settings::Mqueue).toString();

    mq_worker = new QMQueue(this);
    mq_worker->connectToQueue(mq, QIODevice::WriteOnly);
    setConnected();
}

void MqueueHandler::doDisconnect()
{
    if (mq_worker) {
        mq_worker->disconnectFromQueue();
        mq_worker->deleteLater();
        mq_worker = Q_NULLPTR;
    }
    setDisconnected();
}
