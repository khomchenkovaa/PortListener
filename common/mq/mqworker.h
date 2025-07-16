#ifndef MQ_WORKER_H
#define MQ_WORKER_H

#include "mq.h"

class QSettings;

/**
 * @brief класс для работы с контейнером очередей posix
 *
 * если нужно работать с набором очередей, то испльзуйте этот класс
 */
class MQWorker : public QObject
{
    Q_OBJECT

public:
    MQWorker(QObject *parent = NULL) : QObject(parent) { }

    /// создать новый обьект MQ и добавить в m_queues (это не означает что будет создана реальная очередь POSIX)
    void createQueueObj(const QString& mq_name);

    /// разрушить обьект MQ и удалить из m_queues
    void destroyQueueObj(const QString& mq_name);

    void openQueue(int index_mq, int open_mode);

    void closeQueue(int i);

    bool newQueue(const QString &mq_name, int open_mode, quint32 msg_size);

    void removeQueue(int i);

    /// закрыть и удалить только реальную POSIX очередь, объект MQ останется
    void removePosixFile(int i);

    /// создать только реальную POSIX очередь, соответствующую объекту MQ[i]
    void createPosixFile(int i, quint32 msg_size);

    /// отправить сообщение в заданную очередь
    bool sendMessage(int i, const QByteArray &ba);

    /// считать сообщение из заданной очереди (в случае ошибки ba будет пустой)
    QByteArray readMessage(int i);

    /// очистить очередь от сообщений
    bool clearMessages(int i);

    void updateState();

    MQ* lastQueue() const {
        return m_queues.isEmpty() ? Q_NULLPTR : m_queues.last();
    }

    MQ* firstQueue() const {
        return m_queues.isEmpty() ? Q_NULLPTR : m_queues.first();
    }

    MQ* queueAt(int i) const {
        return (i < 0 || i >= count()) ? Q_NULLPTR : m_queues.at(i);
    }

    /// проверить, существует ли очередь с таким именем
    bool queueContains(const QString& mq_name) const {
        return mq_name.isEmpty() ? false : (getMQByName(mq_name) != Q_NULLPTR);
    }

    bool isEmpty() const {
        return m_queues.isEmpty();
    }

    int count() const {
        return m_queues.count();
    }

    void reset() {
        qDeleteAll(m_queues);
        m_queues.clear();
    }

    static QString mqLinuxDir() {
        return "/dev/mqueue";
    }

Q_SIGNALS:
    void logMessage(const QString &msgHandler, const QString &msg);
    void logError(const QString &msgHandler, const QString &msg);

private:
    /// выдать элемент из m_queues по имени очереди
    MQ* getMQByName(const QString& mq_name) const;

private:
    QList<MQ*> m_queues;
};

#endif
