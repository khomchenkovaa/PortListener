#ifndef QMQUEUE_H
#define QMQUEUE_H

#include <QIODevice>
#include <QAbstractSocket>

QT_BEGIN_NAMESPACE

struct QMQueuePrivate;

class QMQueue : public QIODevice
{
    Q_OBJECT

public:
    enum MQueueError {
        ConnectionRefusedError          = QAbstractSocket::ConnectionRefusedError,
        MQueueClosedError               = QAbstractSocket::RemoteHostClosedError,
        MQueueNotFoundError             = QAbstractSocket::HostNotFoundError,
        MQueueAccessError               = QAbstractSocket::SocketAccessError,
        MQueueResourceError             = QAbstractSocket::SocketResourceError,
        MQueueTimeoutError              = QAbstractSocket::SocketTimeoutError,
        DatagramTooLargeError           = QAbstractSocket::DatagramTooLargeError,
        ConnectionError                 = QAbstractSocket::NetworkError,
        UnsupportedMQueueOperationError = QAbstractSocket::UnsupportedSocketOperationError,
        UnknownMQueueError              = QAbstractSocket::UnknownSocketError,
        OperationError                  = QAbstractSocket::OperationError
    };

    enum MQueueState {
        UnconnectedState = QAbstractSocket::UnconnectedState,
        ConnectingState  = QAbstractSocket::ConnectingState,
        ConnectedState   = QAbstractSocket::ConnectedState,
        ClosingState     = QAbstractSocket::ClosingState
    };

    explicit QMQueue(QObject *parent = nullptr);
    ~QMQueue();

    bool connectToQueue(OpenMode mode = ReadWrite);
    bool connectToQueue(const QString &name, OpenMode mode = ReadWrite);
    void disconnectFromQueue();

    void setQueueName(const QString &name);
    QString queueName() const;
    QString fullQueueName() const;

    bool isSequential() const override;
    qint64 bytesAvailable() const override;
    bool open(OpenMode openMode = ReadWrite) override;
    void close() override;
    MQueueState state() const;

Q_SIGNALS:
    void connected();
    void disconnected();
    void errorOccurred(QMQueue::MQueueError queueError);
    void stateChanged(QMQueue::MQueueState socketState);

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char* data, qint64 maxSize) override;

private:
    Q_DISABLE_COPY(QMQueue)

private:
    QSharedPointer<QMQueuePrivate> d;
};

#ifndef QT_NO_DEBUG_STREAM
#include <QtCore/qdebug.h>
QDebug operator<<(QDebug, QMQueue::MQueueError);
QDebug operator<<(QDebug, QMQueue::MQueueState);
#endif

QT_END_NAMESPACE

#endif // QMQUEUE_H
