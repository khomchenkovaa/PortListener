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
    stream.readRawData(reserve, sizeof (reserve));
    stream >> len;
    stream >> cs;
}

/********************************************************/

void DEPHeader::toStream(QDataStream &stream, bool without_cs)
{
    stream << module;
    stream.writeRawData(reserve, sizeof (reserve));
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
    //prepare internal header fields
    headerSize = DEPInternalHeader::REC_SIZE + sizeof (w32_time_us);
    version    = quint32(DEP_PARAM_PACK_VERSION);
    packType   = ptIndividual;
    dataType   = p_type;
    commonTime = tptUTmsecUTC;
    paramTime  = tptUTmsecUTC;
    startIndex = pos;
    paramCount = p_count;
}

/********************************************************/

void DEPDataRecord::fromDataStream(QDataStream &stream, quint32 dataType)
{
    stream >> pack_index;
    w32_time_us t;
    t.fromStream(stream);
    t.dwLow *= 1000;
    dt = t.toQDateTime();
    switch (dataType) {
    case DEPDataType::dpdtFloatValid:
    {
        float val;
        stream >> val;
        value = val;
    } break;
    case DEPDataType::dpdtSDWordValid:
    {
        qint32 val;
        stream >> val;
        value = val;
    }   break;
    default:
        stream.skipRawData(4);
        break;
    }
    stream >> validity;
}

/********************************************************/
