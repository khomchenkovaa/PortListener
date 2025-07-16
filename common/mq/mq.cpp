#include "mq.h"

#include "mqworker.h"

#include <QFileInfo>

#include <mqueue.h>
#include <cstring>

enum {
    MSG_PRIOR = 1
};

int mqMode(int qIODeviceMode)
{
    switch (qIODeviceMode) {
    case QIODevice::WriteOnly: return O_WRONLY;
    case QIODevice::ReadWrite: return O_RDWR;
    default: break;
    }
    return O_RDONLY;
}

MQ::MQ(const QString &name, QObject *parent)
    : QObject(parent)
{
     d.mq_name = name;
     if (!d.mq_name.startsWith('/')) d.mq_name.prepend('/');
}

QString MQ::strStatus() const
{
    if (d.handle > 0)    return QString("handle = %1").arg(d.handle);
    if (d.handle == -99) return "???";
    return QString("err = %1").arg(d.handle);
}

QString MQ::strState() const
{
    switch (d.state) {
    case mqsOpened:   return "Opened";
    case mqsCreated:  return "Created";
    case mqsClosed:   return "Closed";
    case mqsInvalid:  return "Invalid";
    case mqsNotFound: return "Not found";
    default: break;
    }
    return "Not init";	
}

QString MQ::strMode() const
{
    switch (d.mode) {
    case QIODevice::NotOpen:   return "None";
    case QIODevice::ReadOnly:  return "Read";
    case QIODevice::WriteOnly: return "Write";
    case QIODevice::ReadWrite: return "Read/Write";
    default: break;
    }
    return "---";	
}

QString MQ::strAttrs() const
{
    if (!d.attrs) return QString("unknown");
    return QString("flag=%1 msg_num=%2/%3 msg_size=%4")
            .arg(d.attrs->mq_flags).arg(d.attrs->mq_curmsgs).arg(d.attrs->mq_maxmsg).arg(d.attrs->mq_msgsize);
}

bool MQ::tryClose()
{
    if (!isOpened()) {
        emit logError("MQ", QString("MQ[%1]: not was opened").arg(name()));
        return false;
    }

    bool result = false;
    mqd_t resultCode = mq_close(d.handle);
    if (resultCode == 0) {
        d.state = mqsClosed;
        d.mode = 0;
        d.handle = -99;
        result = true;
    } else {
        d.state = mqsInvalid;
    	QString err(std::strerror(errno));
        QString msg = QString("MQ[%1]: closing error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(err);
        emit logError("MQ", msg);
    }
	updateAttrs();
    return result;
}

bool MQ::tryCreate(int mode, quint32 msg_size)
{
    if (existPosixFile()) {
        //file mqueue POSIX allready exist
        emit logError("MQ", QString("POSIX file [%1] allready exist").arg(name()));
        return false;
    }
    if (msg_size < 1) {
        emit logError("MQ", QString("MQ[%1]: creating error, msg_size=%2").arg(name()).arg(msg_size));
        return false;
    }

    bool result = false;
    struct mq_attr attr = {0, 0, 0, 0, {0}};
    attr.mq_flags   = 0; 		// value: 0 or O_NONBLOCK
    attr.mq_maxmsg  = 10; 	    // max number of messages allowed on queue
    attr.mq_curmsgs = 0; 	    // messages count currently in queue
    attr.mq_msgsize = msg_size; // max size of one message

    d.mode = mode;
    int flag = (mqMode(d.mode) | O_CREAT);
    d.handle = mq_open(qPrintable(d.mq_name), flag, 0777, &attr);
    if (d.handle <= 0) {
    	QString err(std::strerror(errno));
        d.state = mqsInvalid;
        QString msg = QString("MQ[%1]: creating error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(err);
        emit logError("MQ", msg);
    } else {
        result = true;
        d.state = mqsOpened;
    }
	updateAttrs();
    return result;
}

void MQ::resetState()
{
    d.state  = mqsDeinit;
    d.mode   = 0;
    d.handle = -99;
    updateAttrs();
}

bool MQ::tryDestroy()
{
    if (!existPosixFile()) {
        resetState();
        return true;
    }
    int result_code = mq_unlink(qPrintable(d.mq_name));
    if (result_code == 0) {
        resetState();
        return true;
    }
    QString err(std::strerror(errno));
    d.state = mqsInvalid;
    QString msg = QString("MQ[%1]: unlink error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(err);
    emit logError("MQ", msg);
	updateAttrs();
    return false;
}

bool MQ::tryOpen(int mode)
{
    checkQueueFile(false);
    if (isNotFound()) {
        return false;
    }
    if (isOpened()) {
        QString msg = QString("MQ[%1]: allready opened").arg(name());
        emit logError("MQ", msg);
        return false;
    }

    bool result = false;
    d.mode = mode;
    d.handle = mq_open(qPrintable(d.mq_name), mqMode(d.mode));
    if (d.handle <= 0) {
    	QString err(std::strerror(errno));
        d.state = mqsInvalid;
        QString msg = QString("MQ[%1]: opening error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(err);
        emit logError("MQ", msg);
    } else {
        result = true;
        d.state = mqsOpened;
    }
	updateAttrs();
    return result;
}

bool MQ::trySendMsg(const QByteArray &ba)
{
    if (!isOpened()) {
        QString msg = QString("MQ[%1]: not was opened").arg(name());
        emit logError("MQ", msg);
        return false;
    }
    if (d.mode != QIODevice::WriteOnly && d.mode != QIODevice::ReadWrite) {
        QString msg = QString("MQ[%1]: open mode(%2) not for writing").arg(name()).arg(d.mode);
        emit logError("MQ", msg);
        return false;
    }
    updateAttrs();
    if (d.attrs && d.attrs->mq_curmsgs >= d.attrs->mq_maxmsg) {
        QString msg = QString("MQ[%1]: stack of queue overflow").arg(name());
        emit logError("MQ", msg);
        return false;
    }

    bool result = false;
    size_t len = ba.count();
    const char *data = ba.data();
    int resultCode = mq_send(d.handle, data, len, MSG_PRIOR);
    if (resultCode == 0) {
        result = true;
    } else {
    	QString err(std::strerror(errno));
        QString msg = QString("MQ[%1]: sending msg error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(err);
        emit logError("MQ", msg);
    }
    updateAttrs();
    return result;
}

QByteArray MQ::tryReadMsg()
{
    if (!isOpened()) {
        QString msg = QString("MQ[%1]: is not opened").arg(name());
        emit logError("MQ", msg);
        return QByteArray();
    }
    if (d.mode != QIODevice::ReadOnly && d.mode != QIODevice::ReadWrite) {
        QString msg = QString("MQ[%1]: open mode(%2) not for reading").arg(name()).arg(d.mode);
        emit logError("MQ", msg);
        return QByteArray();
    }

    updateAttrs();
    if (d.attrs == Q_NULLPTR || d.attrs->mq_curmsgs == 0) {
        QString msg = QString("MQ[%1]: stack of queue is empty").arg(name());
        emit logError("MQ", msg);
        return QByteArray();
    }

    uint prior = 0;
    size_t len = d.attrs->mq_msgsize;
    QByteArray ba;
    ba.fill(0, len);
    int resultCode = mq_receive(d.handle, ba.data(), len, &prior);
    if (resultCode < 0) {
    	QString err(std::strerror(errno));
        QString msg = QString("MQ[%1]: receive msg error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(err);
        emit logError("MQ", msg);
    	ba.clear();
    } else {
        if (ba.size() > resultCode) ba.truncate(resultCode);
        QString msg = QString("MQ[%1]: received msg, size=%2, prior=%3").arg(name()).arg(resultCode).arg(prior);
        emit logMessage("MQ", msg);
    }
    updateAttrs();
    return ba;
}

bool MQ::tryClearAllMsgs()
{
    if (!isOpened()) {
        QString msg = QString("MQ[%1]: not was opened").arg(name());
        emit logError("MQ", msg);
        return false;
    }
    updateAttrs();
    if (d.attrs == Q_NULLPTR || d.attrs->mq_curmsgs == 0) {
        QString msg = QString("MQ[%1]: stack of queue is empty, not necessary for clearing").arg(name());
        emit logMessage("MQ", msg);
        return true;
    }

    //reading msgs
    bool result = true;
    int n_msg = d.attrs->mq_curmsgs;
    for (int i=0; i<n_msg; i++) {
        QByteArray ba = tryReadMsg();
        if (ba.isEmpty()) {
            result = false;
            break;
        }
    }
    return result;
}

void MQ::updateAttrs()
{
    d.size = -1;
    checkQueueFile();
    if (invalid()) return;

    if (!isOpened()) {
        if (d.attrs) {
            delete d.attrs;
            d.attrs = Q_NULLPTR;
        }
        return;
    }

    d.attrs = new mq_attr();
    if (mq_getattr(d.handle , d.attrs) != 0) {
        delete d.attrs;
        d.attrs = Q_NULLPTR;

        QString err(std::strerror(errno));
        QString msg = QString("MQ[%1]: get mq_attrs error, errno=%2, err_msg: %3").arg(name()).arg(errno).arg(err);
        emit logError("MQ", msg);
    }
    d.size = d.attrs->mq_curmsgs * d.attrs->mq_msgsize;
}

bool MQ::hasMsg() const
{
    if (invalid()) return false;
    if (!isOpened() || !d.attrs) return false;
    return (d.attrs->mq_curmsgs > 0);
}

void MQ::checkQueueFile(bool check_invalid)
{
    if (check_invalid && invalid()) return;

    if (!existPosixFile()) {
        if (isOpened()) tryClose();
        if (d.attrs) {
            delete d.attrs;
            d.attrs = Q_NULLPTR;
        }
        d.state = mqsNotFound;
        d.handle = -1;
    }
}

bool MQ::existPosixFile() const
{
    const QString fname = MQWorker::mqLinuxDir() + d.mq_name;
    return QFileInfo::exists(fname);
}
