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
        QByteArray body_ba(d.buffer.mid(DEPHeader::size(), header.len));
        parseBodyPacket(body_ba);
    }
//    d.buffer.remove(0, header.packetSize());
    d.buffer.clear();
}

DEPHeader DEPWorker::tryGetHeader(bool &ok)
{
    DEPHeader result;
    ok = false;

    if (d.buffer.size() < DEPHeader::size()) {
        emit signalError("DEPWorker: WARNING buff size < header size");
        return result;
    }

    while (!ok && d.buffer.size() >= DEPHeader::size()) {
        const auto header = reinterpret_cast<const DEPHeader*>(d.buffer.data());
        if (!header->isModuleValid()) {
            emit signalError(QString("DEPWorker: invalid package header, %1")
                             .arg(header->toString()));
            // в случае ошибки чтения заголовка надо отрезать первое значение для повторной попытки
            d.buffer.remove(0, sizeof(quint32));
            continue;
        }
        if (!header->isChecksumOK()) {
            emit signalError(QString("DEPWorker: invalid header checksum, %1")
                             .arg(header->toString()));
            // в случае ошибки чтения заголовка надо отрезать первое значение для повторной попытки
            d.buffer.remove(0, sizeof(quint32));
            continue;
        }
        // внешний заголовок успешно считан и валиден
        emit signalMsg(QString("DEPWorker: header received, %1")
                         .arg(header->toString()));
        result.module = header->module;
        result.len    = header->len;
        result.cs     = header->cs;
        ok = true;
    }
    return result;
}

bool DEPWorker::isPackChecksumOk(const DEPHeader &header)
{
    if (d.buffer.size() < (int)header.packetSize()) {
        emit signalError(QString("DEPWorker: buffer size (%1) is less than packet size (%2)")
                         .arg(d.buffer.size()).arg(header.packetSize()));
        if (d.buffer.size() < (int)header.packetSizeWithoutCs()) {
            emit signalError(QString("DEPWorker: buffer size (%1) is less than packet size without cs (%2)")
                             .arg(d.buffer.size()).arg(header.packetSizeWithoutCs()));
            return false;
        } else {
            emit signalMsg("DEPWorker: parse package without checksum");
            return true;
        }
    }

    // размер всего полного пакета без КС в конце него
    quint32 checksum = qChecksum(d.buffer.data(), header.packetSizeWithoutCs());
    // КС в конце пакета
    quint32 packCS = *reinterpret_cast<quint32*>(d.buffer.data() + header.packetSizeWithoutCs());
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
    d.lastSigType = -1;
    if (ba.size() < DEPInternalHeader::size()) {
        emit signalError(QString("DEPWorker: too small body size(%1) < depInternalHeaderSize(%2)")
                         .arg(ba.size()).arg(DEPInternalHeader::size()));
        return;
    }

    emit signalMsg(QString("DEPWorker: packet OK. Internal_pack_size %1").arg(ba.size()));

    //read internal header
    const DEPInternalHeader* intHeader = reinterpret_cast<const DEPInternalHeader*>(ba.data());
    emit signalMsg(QString("DEPWorker: internal header received, %1")
                     .arg(intHeader->toString()));

    // read optional data
    if (int(intHeader->headerSize) > DEPInternalHeader::size()) {
        auto time_data = ba.mid(DEPInternalHeader::size(), sizeof (w32_time_us));
        w32_time_us t;
        QDataStream stream(time_data);
        stream.setByteOrder(d.byteOrder);
        t.fromStream(stream);
        t.dwLow *= 1000;
        emit signalMsg(QString("DEPWorker: internal header timepoint, %1").arg(t.toStr()));
    }

    //read parameters data
    parseDataPacket(ba, *intHeader);
}

void DEPWorker::parseDataPacket(const QByteArray &ba, const DEPInternalHeader &i_header)
{
    if (i_header.packType == ptIndividual) {
        d.lastSigType = i_header.dataType;
        if (i_header.paramCount == 0) {
            emit signalError(QString("DEPWorker: in internal header param_count == 0"));
            return;
        }
        switch (i_header.dataType) {
        case dpdtFloatValid:
            readFloatValidData(ba, i_header);
            break;
        case dpdtSDWordValid:
            readSDWordValidData(ba, i_header);
            break;
        default:
            break;
        }
    }
}

void DEPWorker::readSDWordValidData(const QByteArray &ba, const DEPInternalHeader &i_header)
{
    quint32 must_size = i_header.paramCount * DEPSDWordValidRecord::REC_SIZE;
    quint32 cur_size  = ba.size() - i_header.headerSize;
    if (must_size != cur_size) {
        emit signalError(QString("DEPWorker: invalid buff records(SDWord) size(%1), must be %2")
                         .arg(cur_size).arg(must_size));
        return;
    }

    QDataStream stream(ba);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(d.byteOrder);
    stream.skipRawData(i_header.headerSize);
    QList<DEPSDWordValidRecord> result;
    for (quint32 i=0; i < i_header.paramCount; i++) {
        DEPSDWordValidRecord rec;
        rec.fromDataStream(stream);
        result << rec;
    }

    emit dataSDWordReceived(result);
}

void DEPWorker::readFloatValidData(const QByteArray &ba, const DEPInternalHeader &i_header)
{
    quint32 must_size = i_header.paramCount * DEPFloatValidRecord::REC_SIZE;
    quint32 cur_size  = ba.size() - i_header.headerSize;
    if (must_size != cur_size) {
        emit signalError(QString("DEPWorker: invalid buff records(Float) size(%1), must be %2")
                         .arg(cur_size).arg(must_size));
        return;
    }

    QDataStream stream(ba);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(d.byteOrder);
    stream.skipRawData(i_header.headerSize);
    QList<DEPFloatValidRecord> result;
    for (quint32 i=0; i < i_header.paramCount; i++) {
        DEPFloatValidRecord rec;
        rec.fromDataStream(stream);
        result << rec;
    }

    emit dataFloatReceived(result);
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
