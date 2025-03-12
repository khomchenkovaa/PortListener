#include "depworker.h"

#include "mathutils.h"
#include "timeutils.h"
#include "depstructs.h"
#include "depenums.h"

#include <QDataStream>
#include <QDebug>

DEPWorker::DEPWorker(QObject *parent)
    : QObject(parent)
{
    setObjectName("depworker_obj");
}

void DEPWorker::addToBuffer(const QByteArray &ba)
{
    d.buffer.append(ba);
    work();
}

void DEPWorker::work(bool try_next)
{
    if (bufferEmpty()) return;
    if (!try_next) {
        emit signalMsg(QString("DEPWorker: try work received bytes, buffer size %1")
                       .arg(buffSize()));
    }
    if (buffSize() < DEPHeader::byteSize()) {
        qWarning("WARNING buff size < header size");
        return;
    }

    bool ok;
    DEPHeader header;
    tryGetHeader(header, ok);
    if (!ok) return;

    tryGetBody(header, ok);
}

void DEPWorker::tryGetHeader(DEPHeader &header, bool &ok)
{
    ok = false;
    if (buffSize() < DEPHeader::byteSize()) return;

    QDataStream stream(d.buffer);
    prepareStream(stream);
    header.fromDataStream(stream);
    if (header.validModule()) {
        bool cs_ok = headerChecksumOk(header);
        if (!cs_ok) {
            emit signalError(QString("DEPWorker: invalid header checksum, %1")
                             .arg(header.toStr()));
            cutOutFirstValue();
            tryGetHeader(header, ok);
        } else {
            // внешний заголовок успешно считан из m_buffer
            ok = true;
        }
    } else {
        cutOutFirstValue();
        tryGetHeader(header, ok);
    }
}

void DEPWorker::tryGetBody(const DEPHeader &header, bool &ok)
{
    //внешний заголовок был успешно считан
    ok = false;
    int p_size = lastPackSize(header);
    if (buffSize() < p_size) {
        //не хватает байт для чтения всего пакета целиком
        emit signalError(QString("DEPWorker: buffer size (%1) < packet size (%2)")
                         .arg(buffSize()).arg(p_size));
        return;
    }

    qDebug() << Qt::endl << QString("DEPHeader OK: %1,  buffSize=%2").arg(header.toStr()).arg(buffSize());

    QDataStream stream(d.buffer);
    prepareStream(stream, DEPHeader::byteSize() + header.len);
    stream >> d.packCS;

    if (packChecksumOk(header)) {
        ok = true;
        // take body bytes
        QByteArray body_ba(d.buffer.mid(DEPHeader::byteSize(), header.len));
        // извлечь данные из тела пакета
        parseBodyPacket(body_ba);
    } else {
        QString checkSum = Utils::uint32ToBAStr(d.packCS, true);
        emit signalError(QString("DEPWorker: invalid checksum of all packet, %1")
                         .arg(checkSum));
        qWarning()<<QString("DEPWorker: WARNING - invalid packet checksum, %1").arg(checkSum);
    }
    // удалить текущий пакет из m_buffer (независимо от правильности КС)
    d.buffer.remove(0, lastPackSize(header));

    if (ok) {
        //если все ок, то пытаемся читать следующий пакет сразу, на случай если в m_buffer есть еще данные
        work(true);
    }
}

bool DEPWorker::headerChecksumOk(const DEPHeader &header) const
{
    if (buffSize() < DEPHeader::byteSize()) return false;
    quint32 cs = qChecksum(d.buffer.data(), DEPHeader::byteSize()-depChecksumSize());
    return (cs == header.cs);
}

bool DEPWorker::packChecksumOk(const DEPHeader &header) const
{
    //размер всего полного пакета без КС в конце него
    int n = DEPHeader::byteSize() + header.len;
    quint32 cs = qChecksum(d.buffer.data(), n);
    return (cs == d.packCS);
}

int DEPWorker::lastPackSize(const DEPHeader &header) const
{
    if (!header.validModule()) return -1;
    return (DEPHeader::byteSize() + header.len + depChecksumSize());
}

void DEPWorker::wrapPacket(QByteArray& packet_ba) const
{
    if(packet_ba.size() < 0) return;

    //prepare stream for writing packet
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    prepareStream(stream);

    //prepare external header
    DEPHeader header(depsParamPacket);
    header.len = packet_ba.size();
    header.toStream(stream, true);
    //calc header cs
    header.cs = qChecksum(ba.data(), ba.size());
    stream << header.cs;

    //add body and cs of all packet
    ba.append(packet_ba);
    stream.skipRawData(header.len);
    header.cs = qChecksum(ba.data(), ba.size());
    stream << header.cs;
    packet_ba.clear();
    packet_ba.append(ba);
}

void DEPWorker::parseBodyPacket(const QByteArray &ba)
{
    //весь пакет был успешно считан, читаем теперь внутренний пакет
    d.lastSigType = -1;
    if (ba.size() < DEPInternalHeader::byteSize()) {
        emit signalError(QString("DEPWorker: too small body size(%1) < depInternalHeaderSize(%2)")
                         .arg(ba.size()).arg(DEPInternalHeader::byteSize()));
        return;
    }

    qDebug()<<QString("full packet OK: internal_pack_size %1, DEPInternalHeader size %2").arg(ba.size()).arg(DEPInternalHeader::byteSize());

    //prepare data stream
    QDataStream body_stream(ba);
    prepareStream(body_stream);

    //read internal header
    DEPInternalHeader i_header;
    i_header.fromDataStream(body_stream);
    qDebug()<<i_header.toStr();
    if (int(i_header.header_len) >  DEPInternalHeader::byteSize()) {
        w32_time_us t;
        t.fromStream(body_stream);
        t.dwLow *= 1000;
        qDebug()<<QString("HEADER TIME_POINT:  ")<<t.toStr();
    }
    //read parameters data
    parseDataPacket(ba, i_header, body_stream);
}

void DEPWorker::parseDataPacket(const QByteArray &ba, const DEPInternalHeader &i_header, QDataStream &stream)
{
    if (i_header.metod == ptIndividual) {
        d.lastSigType = i_header.data_type;
        if (i_header.param_count == 0) {
            emit signalError(QString("DEPWorker: in internal header param_count == 0"));
            return;
        }
        switch (i_header.data_type) {
        case dpdtFloatValid:
            readFloatValidData(ba, i_header, stream);
            break;
        case dpdtSDWordValid:
            readSDWordValidData(ba, i_header, stream);
            break;
        default:
            break;
        }
    }
}

void DEPWorker::readSDWordValidData(const QByteArray &ba, const DEPInternalHeader &i_header, QDataStream &stream)
{
    quint32 must_size = i_header.param_count * DEPSDWordValidRecord::REC_SIZE;
    quint32 cur_size  = ba.size() - i_header.header_len;
    if (must_size != cur_size) {
        emit signalError(QString("DEPWorker: invalid buff records(SDWord) size(%1), must be %2")
                         .arg(cur_size).arg(must_size));
        return;
    }

    QByteArray view_ba;     ///< массив байт предназначенный для вьюхи, туда будут порциями(8 байт) складываться значения и валидности
    QList<quint16> indexes; ///< список индексов-позиций параметров в пакете DEP для сопоставления с местом положения во вьюхе
    for (int i=0; i<int(i_header.param_count); i++) {
        DEPSDWordValidRecord rec;
        rec.fromDataStream(stream);
        //позиция текущей записи(offset)
        int rec_pos = i_header.header_len + DEPSDWordValidRecord::REC_SIZE*i;
        view_ba.append(ba.mid(rec_pos+12, VIEW_PARAM_SIZE));
        indexes.append(rec.pack_index);
    }

    emit signalRewriteReceivedPacket(indexes, view_ba);
}

void DEPWorker::readFloatValidData(const QByteArray &ba, const DEPInternalHeader &i_header, QDataStream &stream)
{
    quint32 must_size = i_header.param_count*DEPFloatValidRecord::REC_SIZE;
    quint32 cur_size  = ba.size() - i_header.header_len;
    if (must_size != cur_size) {
        emit signalError(QString("DEPWorker: invalid buff records(Float) size(%1), must be %2")
                         .arg(cur_size).arg(must_size));
        return;
    }

    QByteArray view_ba;     ///< массив байт предназначенный для вьюхи, туда будут порциями(8 байт) складываться значения и валидности
    QList<quint16> indexes; ///< список индексов-позиций параметров в пакете DEP для сопоставления с местом положения во вьюхе
    for (int i=0; i<int(i_header.param_count); i++) {
        DEPFloatValidRecord rec;
        rec.fromDataStream(stream);
        // позиция текущей записи(offset)
        int rec_pos = i_header.header_len + DEPFloatValidRecord::REC_SIZE*i;
        view_ba.append(ba.mid(rec_pos+12, VIEW_PARAM_SIZE));
        indexes.append(rec.pack_index);
    }

    emit signalRewriteReceivedPacket(indexes, view_ba);
}

QByteArray DEPWorker::makeIntValidPacket(const QList<quint16> &indexes, const QByteArray &view_ba) const
{
    return makeDEPPacket(indexes, view_ba, dpdtSDWordValid);
}

QByteArray DEPWorker::makeFloatValidPacket(const QList<quint16> &indexes, const QByteArray &view_ba) const
{
    return makeDEPPacket(indexes, view_ba, dpdtFloatValid);
}

QByteArray DEPWorker::makeDEPPacket(const QList<quint16> &indexes, const QByteArray &view_ba, int p_type) const
{
    //prepare data stream for pack BA
    QByteArray packet_ba;
    QDataStream body_stream(&packet_ba, QIODevice::WriteOnly);
    prepareStream(body_stream);

    //prepare internal header
    DEPInternalHeader i_header;
    i_header.prepare(p_type, indexes.count());
    i_header.toStream(body_stream);

    //prepare time
    w32_time_us t;
    t.setCurrentTime();
    t.dwLow /= 1000;
    t.toStream(body_stream);

    //write params data
    for (int i=0; i<indexes.count(); i++) {
        //qDebug()<<QString("float params: index %1").arg(indexes.at(i));
        body_stream << quint32(indexes.at(i));
        t.toStream(body_stream);
        packet_ba.append(view_ba.mid(VIEW_PARAM_SIZE*i, VIEW_PARAM_SIZE));
        body_stream.skipRawData(VIEW_PARAM_SIZE);
    }
    //make full DEP packet
    wrapPacket(packet_ba);
    return packet_ba;
}

void DEPWorker::prepareStream(QDataStream &stream, int skip_bytes) const
{
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder((QDataStream::ByteOrder)d.byteOrder);
    if (skip_bytes > 0) stream.skipRawData(skip_bytes);
}

void DEPWorker::cutOutFirstValue()
{
    d.buffer.remove(0, depChecksumSize());
}
