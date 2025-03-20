#include "depworker.h"

#include "mathutils.h"
#include "timeutils.h"

#include <QDebug>

void DEPWorker::work(bool try_next)
{
    if (d.buffer.isEmpty()) return;
    if (!try_next) {
        emit signalMsg(QString("DEPWorker: try work received bytes, buffer size is %1")
                       .arg(d.buffer.size()));
    }

    bool ok;
    DEPHeader header = tryGetHeader(ok);
    if (!ok) return;
    if (isPackChecksumOk(header)) {
        QByteArray body_ba(d.buffer.mid(DEPHeader::REC_SIZE, header.len));
        parseBodyPacket(body_ba);
    }
//    d.buffer.remove(0, header.packetSize());
    d.buffer.clear();
}

DEPHeader DEPWorker::tryGetHeader(bool &ok)
{
    DEPHeader header;
    ok = false;

    if (d.buffer.size() < DEPHeader::REC_SIZE) {
        emit signalError("DEPWorker: WARNING buff size < header size");
        return header;
    }

    while (!ok && d.buffer.size() >= DEPHeader::REC_SIZE) {
        QDataStream stream(d.buffer);
        stream.setByteOrder(d.byteOrder);
        stream.setFloatingPointPrecision(d.precision);
        header.fromDataStream(stream);
        if (!header.isModuleValid()) {
            emit signalError(QString("DEPWorker: invalid package header, %1")
                             .arg(header.toString()));
            // в случае ошибки чтения заголовка надо отрезать первое значение для повторной попытки
            d.buffer.remove(0, sizeof(quint32));
            continue;
        }
        quint32 cs = qChecksum(d.buffer.data(), DEPHeader::REC_SIZE - sizeof (quint32));
        if (header.cs != cs) {
            emit signalError(QString("DEPWorker: invalid header checksum, %1")
                             .arg(header.toString()));
            // в случае ошибки чтения заголовка надо отрезать первое значение для повторной попытки
            d.buffer.remove(0, sizeof(quint32));
            continue;
        }
        // внешний заголовок успешно считан и валиден
        emit signalMsg(QString("DEPWorker: %1").arg(header.toString()));
        ok = true;
    }
    return header;
}

bool DEPWorker::isPackChecksumOk(const DEPHeader &header)
{
    if (d.buffer.size() < (int)header.packetSize()) {
        emit signalError(QString("DEPWorker: buffer size (%1) is less than packet size (%2)")
                         .arg(d.buffer.size()).arg(header.packetSize()));
        return false;
    }

    // размер всего полного пакета без КС в конце него
    quint32 checksum = qChecksum(d.buffer.data(), header.packetSizeWithoutCs());
    // КС в конце пакета
    quint32 packCS;
    QDataStream stream(d.buffer);
    stream.setByteOrder(d.byteOrder);
    stream.setFloatingPointPrecision(d.precision);
    stream.skipRawData(header.packetSizeWithoutCs());
    stream >> packCS;
    if (checksum != packCS) {
        emit signalError(QString("DEPWorker: invalid checksum of all packet, 0x%1").arg(packCS, 0, 16));
        return false;
    }
    return true;
}

void DEPWorker::wrapPacket(QByteArray& packet_ba) const
{
    if(packet_ba.size() < 0) return;

    //prepare stream for writing packet
    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(d.byteOrder);

    //prepare external header
    DEPHeader header(depsParamPacket);
    header.len = packet_ba.size();
    header.toStream(stream, true);
    //calc header cs
    header.cs = qChecksum(ba.data(), ba.size());
    stream << header.cs;

    //add body of all packet
    stream.writeRawData(packet_ba.data(), packet_ba.size());

    //add cs of all packet
    quint32 cs = qChecksum(ba.data(), ba.size());
    stream << cs;
    packet_ba.clear();
    packet_ba.append(ba);
}

void DEPWorker::parseBodyPacket(const QByteArray &ba)
{
    //весь пакет был успешно считан, читаем теперь внутренний пакет
    if (ba.size() < DEPInternalHeader::REC_SIZE) {
        emit signalError(QString("DEPWorker: too small body size(%1) < depInternalHeaderSize(%2)")
                         .arg(ba.size()).arg(DEPInternalHeader::REC_SIZE));
        return;
    }

    //read internal header
    QDataStream stream(ba);
    stream.setByteOrder(d.byteOrder);
    stream.setFloatingPointPrecision(d.precision);
    DEPInternalHeader intHeader;
    intHeader.fromDataStream(stream);
    emit signalMsg(QString("DEPWorker: %1").arg(intHeader.toString()));

    // read optional data
    if (int(intHeader.headerSize) > DEPInternalHeader::REC_SIZE) {
        w32_time_us t;
        t.fromStream(stream);
        t.dwLow *= 1000;
        emit signalMsg(QString("DEPWorker: %1").arg(t.toStr()));
    }

    //read parameters data
    parseDataPacket(ba, intHeader);
}

void DEPWorker::parseDataPacket(const QByteArray &ba, const DEPInternalHeader &i_header)
{
    if (i_header.packType == ptIndividual) {
        if (i_header.paramCount == 0) {
            emit signalError(QString("DEPWorker: in internal header param_count == 0"));
            return;
        }
        readData(ba, i_header);
    }
}

void DEPWorker::readData(const QByteArray &ba, const DEPInternalHeader &i_header)
{
    quint32 must_size = i_header.paramCount * DEPDataRecord::REC_SIZE;
    quint32 cur_size  = ba.size() - i_header.headerSize;
    if (must_size != cur_size) {
        emit signalError(QString("DEPWorker: invalid buff records size(%1), must be %2")
                         .arg(cur_size).arg(must_size));
        return;
    }

    QDataStream stream(ba);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(d.byteOrder);
    stream.skipRawData(i_header.headerSize);
    QList<DEPDataRecord> result;
    for (quint32 i=0; i < i_header.paramCount; i++) {
        DEPDataRecord rec;
        rec.fromDataStream(stream, i_header.dataType);
        result << rec;
    }

    emit dataReceived(result);
}

QByteArray DEPWorker::makeDEPPacket(const QList<quint16> &indexes, const QByteArray &view_ba, int p_type) const
{
    //prepare data stream for pack BA
    QByteArray packet_ba;
    QDataStream body_stream(&packet_ba, QIODevice::WriteOnly);
    body_stream.setFloatingPointPrecision(d.precision);
    body_stream.setByteOrder(d.byteOrder);

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
        body_stream << quint32(indexes.at(i));
        t.toStream(body_stream);
        body_stream.writeRawData(view_ba.mid(VIEW_PARAM_SIZE*i, VIEW_PARAM_SIZE).data(), VIEW_PARAM_SIZE);
    }
    //make full DEP packet
    wrapPacket(packet_ba);
    return packet_ba;
}
