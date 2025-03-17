#include "depstructs.h"
#include "mathutils.h"
#include "timeutils.h"

#include <QDataStream>
#include <QDebug>

/********************************************************/

QString DEPHeader::toString() const
{
    return QString("DEPHeader: module=0x%1 len=%2 cs=%3")
            .arg(QString::number(module, 16))
            .arg(len)
            .arg(Utils::uint32ToBAStr(cs, true));
}

/********************************************************/

void DEPHeader::fromDataStream(QDataStream &stream)
{
    stream >> module;
    qint8 a = -1;
    for (int i=0; i<20; i++) {
        stream >> a;
        reserve[i] = a;
    }
    stream >> len;
    stream >> cs;
}

/********************************************************/

void DEPHeader::toStream(QDataStream &stream, bool without_cs)
{
    stream << module;
    for (int i=0; i<20; i++) stream << qint8(reserve[i]);
    stream << len;
    if (!without_cs) stream << cs;
}

/********************************************************/

QString DEPInternalHeader::toString() const
{
    return QString("DEPInternalHeader: header_len=%1  VERSION(%2)  metod=%3  data_type=%4  common_time=%5  param_time=%6  start_index=%7  param_count=%8").
            arg(headerSize).arg(Utils::uint32ToBAStr(version, true)).arg(packType).arg(dataType).
            arg(commonTime).arg(paramTime).arg(startIndex).arg(paramCount);
}

/********************************************************/

void DEPInternalHeader::fromDataStream(QDataStream &stream)
{
    stream >> headerSize >> version   >> packType   >> dataType
           >> commonTime >> paramTime >> startIndex >> paramCount;
}

/********************************************************/

void DEPInternalHeader::toStream(QDataStream &stream)
{
    stream << headerSize << version   << packType   << dataType
           << commonTime << paramTime << startIndex << paramCount;
}

/********************************************************/

void DEPInternalHeader::prepare(int p_type, int p_count, quint32 pos)
{
    w32_time_us t;

    //prepare internal header fields
    headerSize = DEPInternalHeader::size() + t.size();
    version    = quint32(DEP_PARAM_PACK_VERSION);
    packType   = ptIndividual;
    dataType   = p_type;
    commonTime = tptUTmsecUTC;
    paramTime  = tptUTmsecUTC;
    startIndex = pos;
    paramCount = p_count;
}

/********************************************************/

void DEPFloatValidRecord::fromDataStream(QDataStream &stream)
{
    stream >> pack_index;
    w32_time_us t;
    t.fromStream(stream);
    t.dwLow *= 1000;
    dt = t.toQDateTime();
    stream >> value >> validity;
}

/********************************************************/

void DEPSDWordValidRecord::fromDataStream(QDataStream &stream)
{
    stream >> pack_index;
    w32_time_us t;
    t.fromStream(stream);
    t.dwLow *= 1000;
    dt = t.toQDateTime();
    stream >> value >> validity;
}

/********************************************************/
