#include "mqworker.h"

#include <QIODevice>

void MQWorker::createQueueObj(const QString &mq_name)
{
	MQ *mq = new MQ(mq_name, this);
	m_queues.append(mq);
    emit logMessage("MQWorker", QString("created MQ object, name: %1").arg(mq->name()));
    connect(mq, &MQ::logError,   this, &MQWorker::logError);
    connect(mq, &MQ::logMessage, this, &MQWorker::logMessage);
}

void MQWorker::destroyQueueObj(const QString &mq_name)
{
    MQ *mq = getMQByName(mq_name);
    if (!mq) {
        emit logError("MQWorker", QString("queue [%1] not found").arg(mq_name));
		return;
	}
	int pos = m_queues.indexOf(mq);
	m_queues.removeAt(pos);
	delete mq;
    emit logMessage("MQWorker", QString("MQ object destroyed, name: %1").arg(mq_name));
}

void MQWorker::openQueue(int index_mq, int open_mode)
{
    if (index_mq < 0 || index_mq >= count()) {
        emit logError("MQWorker", QString("MQWorker: invalid queue index %1").arg(index_mq));
		return;
	}
    bool ok = m_queues.at(index_mq)->tryOpen(open_mode);
    if (ok) {
        emit logMessage("MQWorker", QString("queue [%1] was opened,  handle=%2").arg(m_queues.at(index_mq)->name()).arg(m_queues.at(index_mq)->handle()));
    }
}

void MQWorker::closeQueue(int i)
{
    if (i < 0 || i >= count()) {
        emit logError("MQWorker", QString("MQWorker: invalid queue index %1").arg(i));
		return;
	}
    bool ok = m_queues.at(i)->tryClose();
    if (ok) {
        emit logMessage("MQWorker", QString("queue [%1] was closed!").arg(m_queues.at(i)->name()));
    }
}

bool MQWorker::newQueue(const QString &mq_name, int open_mode, quint32 msg_size)
{
	createQueueObj(mq_name);
    bool ok = m_queues.last()->tryCreate(open_mode, msg_size);
    if (ok) {
        emit logMessage("MQWorker", QString("queue [%1] was created!").arg(m_queues.last()->name()));
    } else {
    	MQ *mq = m_queues.takeLast();
    	if (mq) delete mq;
        emit logMessage("MQWorker", QString("last MQ object destroyed, queues size %1").arg(m_queues.count()));
    }
    return ok;
}

void MQWorker::createPosixFile(int i, quint32 msg_size)
{
    if (i < 0 || i >= count()) {
        emit logError("MQWorker", QString("invalid queue index %1").arg(i));
        return;
    }
    bool ok = m_queues.at(i)->tryCreate(0, msg_size);
    if (ok) {
        emit logMessage("MQWorker", QString("POSIX file [%1] was created!").arg(m_queues.at(i)->name()));
        ok = m_queues.at(i)->tryClose();
        if (ok) m_queues.at(i)->resetState();
    }
}

void MQWorker::removePosixFile(int i)
{
    if (i < 0 || i >= count()) {
        emit logError("MQWorker", QString("invalid queue index %1").arg(i));
        return;
    }
    m_queues.at(i)->tryDestroy();
}

void MQWorker::removeQueue(int i)
{
    if (i < 0 || i >= count()) {
        emit logError("MQWorker", QString("MQWorker: invalid queue index %1").arg(i));
		return;
	}
    if (m_queues.at(i)->tryDestroy()) {
        destroyQueueObj(m_queues.at(i)->name());
    }
}

bool MQWorker::sendMessage(int i, const QByteArray &ba)
{
    if (i < 0 || i >= count()) {
        emit logError("MQWorker", QString("MQWorker: invalid queue index %1").arg(i));
        return false;
	}

    emit logMessage("MQWorker", QString("try send pack (%1 bytes) to queue: [%2]").arg(ba.size()).arg(m_queues.at(i)->name()));

    bool result = m_queues.at(i)->trySendMsg(ba);
    if (result) {
        emit logMessage("MQWorker", "message was sended");
    }
    return result;
}

QByteArray MQWorker::readMessage(int i)
{
    if (i < 0 || i >= count()) {
        emit logError("MQWorker", QString("MQWorker: invalid queue index %1").arg(i));
        return QByteArray();
	}
    QByteArray result = m_queues.at(i)->tryReadMsg();
    if (!result.isEmpty()) {
        emit logMessage("MQWorker", QString("readed %1 bytes").arg(result.count()));
	}
    return result;
}

bool MQWorker::clearMessages(int i)
{
    if (i < 0 || i >= count()) {
        emit logError("MQWorker", QString("MQWorker: invalid queue index %1").arg(i));
        return false;
    }
    bool result = m_queues.at(i)->tryClearAllMsgs();
    if (result) {
        emit logMessage("MQWorker", QString("clearing [%1] done!").arg(m_queues.at(i)->name()));
    }
    return result;
}

void MQWorker::updateState()
{
    for (MQ *value : qAsConst(m_queues)) {
        if (value) value->updateAttrs();
	}
}

MQ* MQWorker::getMQByName(const QString &mq_name) const
{
    MQ *result = Q_NULLPTR;
    QString name = mq_name;
    if (!name.startsWith('/')) name.prepend('/');
    for (MQ *item : m_queues) {
        if (item->name() == name) {
            result = item;
			break;
		}
	}
    return result;
}
