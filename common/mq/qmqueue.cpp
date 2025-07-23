#include "qmqueue.h"

#include <mqueue.h>
#include <cstring>

#include <QTimer>

enum {
    MSG_PRIOR = 1
};

QT_BEGIN_NAMESPACE

QString generateErrorString(QMQueue::MQueueError error, const QString &function)
{
    QString errorString;
    switch (error) {
    case QMQueue::ConnectionRefusedError:
        errorString = QMQueue::tr("%1: Connection refused").arg(function);
        break;
    case QMQueue::MQueueClosedError:
        errorString = QMQueue::tr("%1: mqueue closed").arg(function);
        break;
    case QMQueue::MQueueNotFoundError:
        errorString = QMQueue::tr("%1: Invalid name").arg(function);
        break;
    case QMQueue::MQueueAccessError:
        errorString = QMQueue::tr("%1: mqueue access error").arg(function);
        break;
    case QMQueue::MQueueResourceError:
        errorString = QMQueue::tr("%1: mqueue resource error").arg(function);
        break;
    case QMQueue::MQueueTimeoutError:
        errorString = QMQueue::tr("%1: mqueue operation timed out").arg(function);
        break;
    case QMQueue::DatagramTooLargeError:
        errorString = QMQueue::tr("%1: Datagram too large").arg(function);
        break;
    case QMQueue::ConnectionError:
        errorString = QMQueue::tr("%1: Connection error").arg(function);
        break;
    case QMQueue::UnsupportedMQueueOperationError:
        errorString = QMQueue::tr("%1: The mqueue operation is not supported").arg(function);
        break;
    case QMQueue::OperationError:
        errorString = QMQueue::tr("%1: Operation not permitted when mqueue is in this state").arg(function);
        break;
    case QMQueue::UnknownMQueueError:
    default:
        errorString = QMQueue::tr("%1: Unknown error %2").arg(function).arg(errno);
    }
    return errorString;
}

int q_mqMode(int qIODeviceMode)
{
    switch (qIODeviceMode) {
    case QIODevice::WriteOnly: return O_CREAT | O_WRONLY;
    case QIODevice::ReadWrite: return O_CREAT | O_RDWR;
    default: break;
    }
    return O_RDONLY;
}

struct QMQueuePrivate {
    QString  queueName;          ///< имя очереди POSIX
    mqd_t    handle = -1;	     ///< дескриптор очереди posix
    QMQueue::MQueueState state = QMQueue::UnconnectedState; ///< MQState enum element
    mq_attr  attrs;              ///< current attributes
    QTimer   timer;

    /// текущий размер очереди
    int size() {
        return attrs.mq_curmsgs * attrs.mq_msgsize;
    }
};

QMQueue::QMQueue(QObject *parent)
    : QIODevice{parent}
{
    d = QSharedPointer<QMQueuePrivate>::create();
}

QMQueue::~QMQueue()
{
    QMQueue::close();
}

bool QMQueue::connectToQueue(OpenMode mode)
{
    if (state() == ConnectedState || state() == ConnectingState) {
        QString errorString = generateErrorString(QMQueue::OperationError, "QMQueue::connectToQueue");
        setErrorString(errorString);
        emit errorOccurred(QMQueue::OperationError);
        return false;
    }

    setErrorString(QString());
    d->handle = -1;
    d->state = ConnectingState;
    emit stateChanged(d->state);

    if (d->queueName.isEmpty()) {
        QString errorString = generateErrorString(QMQueue::MQueueNotFoundError, "QMQueue::connectToQueue");
        setErrorString(errorString);
        emit errorOccurred(QMQueue::MQueueNotFoundError);

        // errors cause a disconnect
        d->state = QMQueue::UnconnectedState;
        emit stateChanged(d->state);
        return false;
    }

    d->handle = mq_open(qPrintable(d->queueName), q_mqMode(mode), 0666, NULL);

    if (d->handle == (mqd_t)-1) {
        QString errorString = generateErrorString(QMQueue::UnsupportedMQueueOperationError, "QMQueue::connectToQueue");
        setErrorString(errorString);
        emit errorOccurred(QMQueue::UnsupportedMQueueOperationError);

        // errors cause a disconnect
        d->state = QMQueue::UnconnectedState;
        emit stateChanged(d->state);
        return false;
    }

    setOpenMode(mode);
    d->state = ConnectedState;
    emit stateChanged(d->state);
    emit connected();

    connect(&d->timer, &QTimer::timeout, this, &QMQueue::checkNewMessages);
    d->timer.start(1000);

    return isOpen();
}

bool QMQueue::connectToQueue(const QString &name, OpenMode mode)
{
    setQueueName(name);
    return connectToQueue(mode);
}

void QMQueue::disconnectFromQueue()
{
    d->state = ClosingState;
    emit stateChanged(d->state);

    d->timer.stop();

    int result_code = mq_close(d->handle);
    setErrorString(result_code ? std::strerror(errno) : QString());
    d->handle = -1;
    d->state = UnconnectedState;
    setOpenMode(QIODevice::NotOpen);

    emit stateChanged(d->state);
    emit disconnected();
}

void QMQueue::setQueueName(const QString &name)
{
    if (d->state != UnconnectedState) {
        qWarning("QMQueue::setQueueName called while not in unconnected state");
        return;
    }
    d->queueName = name;
    if (!d->queueName.startsWith('/')) d->queueName.prepend('/');
}

QString QMQueue::queueName() const
{
    return d->queueName;
}

bool QMQueue::isSequential() const
{
    return true;
}

qint64 QMQueue::bytesAvailable() const
{
    if (mq_getattr(d->handle , &(d->attrs)) != 0) {
        return 0;
    }
    return d->size();
}

bool QMQueue::open(OpenMode openMode)
{
    return connectToQueue(openMode);
}

void QMQueue::close()
{
    disconnectFromQueue();
}

QMQueue::MQueueState QMQueue::state() const
{
    return d->state;
}

qint64 QMQueue::readData(char *data, qint64 maxSize)
{
    Q_UNUSED(maxSize)
    if (mq_getattr(d->handle, &(d->attrs))) {
         setErrorString(std::strerror(errno));
         return -1;
    }
    uint prior = MSG_PRIOR;
    qint64 maxLen = qMin<qint64>(maxSize, d->attrs.mq_msgsize);
    int resultCode = mq_receive(d->handle, data, maxLen, &prior);
    if (resultCode) {
        setErrorString(std::strerror(errno));
        return -1;
    }
    setErrorString(QString());
    return qstrlen(data);
}

qint64 QMQueue::writeData(const char *data, qint64 maxSize)
{
    if (mq_getattr(d->handle, &(d->attrs))) {
         setErrorString(std::strerror(errno));
         return -1;
    }
    qint64 maxLen = qMin<qint64>(maxSize, d->attrs.mq_msgsize);
    maxLen = qMin<qint64>(maxLen, qstrlen(data));
    int resultCode = mq_send(d->handle, data, maxLen, MSG_PRIOR);
    if (resultCode) {
        setErrorString(std::strerror(errno));
        return -1;
    }
    setErrorString(QString());
    return maxLen;
}

void QMQueue::checkNewMessages()
{
    if (mq_getattr(d->handle , &(d->attrs)) == 0) {
        if (d->attrs.mq_curmsgs) {
            emit readyRead();
        }
    }
}


#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, QMQueue::MQueueError error)
{
    QDebugStateSaver saver(debug);
    debug.resetFormat().nospace();
    switch (error) {
    case QMQueue::ConnectionRefusedError:
        debug << "QMQueue::ConnectionRefusedError";
        break;
    case QMQueue::MQueueClosedError:
        debug << "QMQueue::MQueueClosedError";
        break;
    case QMQueue::MQueueNotFoundError:
        debug << "QMQueue::MQueueNotFoundError";
        break;
    case QMQueue::MQueueAccessError:
        debug << "QMQueue::MQueueAccessError";
        break;
    case QMQueue::MQueueResourceError:
        debug << "QMQueue::MQueueResourceError";
        break;
    case QMQueue::MQueueTimeoutError:
        debug << "QMQueue::MQueueTimeoutError";
        break;
    case QMQueue::DatagramTooLargeError:
        debug << "QMQueue::DatagramTooLargeError";
        break;
    case QMQueue::ConnectionError:
        debug << "QMQueue::ConnectionError";
        break;
    case QMQueue::UnsupportedMQueueOperationError:
        debug << "QMQueue::UnsupportedMQueueOperationError";
        break;
    case QMQueue::UnknownMQueueError:
        debug << "QMQueue::UnknownMQueueError";
        break;
    case QMQueue::OperationError:
        debug << "QMQueue::OperationError";
        break;
    default:
        debug << "QMQueue::SocketError(" << int(error) << ')';
        break;
    }
    return debug;
}

QDebug operator<<(QDebug debug, QMQueue::MQueueState state)
{
    QDebugStateSaver saver(debug);
    debug.resetFormat().nospace();
    switch (state) {
    case QMQueue::UnconnectedState:
        debug << "QMQueue::UnconnectedState";
        break;
    case QMQueue::ConnectingState:
        debug << "QMQueue::ConnectingState";
        break;
    case QMQueue::ConnectedState:
        debug << "QMQueue::ConnectedState";
        break;
    case QMQueue::ClosingState:
        debug << "QMQueue::ClosingState";
        break;
    default:
        debug << "QMQueue::SocketState(" << int(state) << ')';
        break;
    }
    return debug;
}
#endif

QT_END_NAMESPACE
