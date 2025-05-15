#include "depstructs.h"
#include "timeutils.h"

#include <QDataStream>
#include <QDebug>

/********************************************************/

QString DEPHeader::toString() const
{
    return QString("DEPHeader: module=0x%1 len=%2 cs=0x%3")
            .arg(module, 8, 16, QLatin1Char('0'))
            .arg(bodySize)
            .arg(headerChecksum, 8, 16, QLatin1Char('0'));
}

/********************************************************/

void DEPHeader::fromDataStream(QDataStream &stream)
{
    stream >> module;
    stream.readRawData(reserve, sizeof (reserve));
    stream >> bodySize;
    stream >> headerChecksum;
}

/********************************************************/

void DEPHeader::toStream(QDataStream &stream, bool without_cs)
{
    stream << module;
    stream.writeRawData(reserve, sizeof (reserve));
    stream << bodySize;
    if (!without_cs) stream << headerChecksum;
}

/********************************************************/

QString DEPDataHeader::toString() const
{
    return QString("DEPInternalHeader: header_len=%1  VERSION(0x%2)  metod=%3  data_type=%4  common_time=%5  param_time=%6  start_index=%7  param_count=%8")
            .arg(headerSize)
            .arg(version, 8, 16, QLatin1Char('0'))
            .arg(packType).arg(dataType)
            .arg(commonTime).arg(paramTime)
            .arg(startIndex).arg(paramCount);
}

/********************************************************/

void DEPDataHeader::fromDataStream(QDataStream &stream)
{
    stream >> headerSize >> version   >> packType   >> dataType
           >> commonTime >> paramTime >> startIndex >> paramCount;
}

/********************************************************/

void DEPDataHeader::toStream(QDataStream &stream)
{
    stream << headerSize << version   << packType   << dataType
           << commonTime << paramTime << startIndex << paramCount;
}

/********************************************************/

void DEPDataHeader::prepare(int p_type, int p_count, quint32 pos)
{
    //prepare internal header fields
    headerSize = DEPDataHeader::REC_SIZE + sizeof (w32_time_us);
    version    = quint32(DEP_PARAM_PACK_VERSION);
    packType   = ptIndividual;
    dataType   = p_type;
    commonTime = dptpUTmsecUTC;
    paramTime  = dptpUTmsecUTC;
    startIndex = pos;
    paramCount = p_count;
}

/********************************************************/

void DEPDataRecord::fromDataStream(QDataStream &stream, const DEPDataHeader &i_header)
{
    stream >> pack_index;
    switch (i_header.paramTime) {
    case DEPTimePoint::dptpNone:
        dt = QDateTime::currentDateTime();
        stream.skipRawData(sizeof(w32_time_us));
        break;
    case DEPTimePoint::dptpUTmsecUTC:
    {
        w32_time_us t;
        t.fromStream(stream);
        t.dwLow *= 1000;
        dt = t.toQDateTime();
    }
        break;
    default: // unsupported
        stream.skipRawData(sizeof(w32_time_us));
        break;
    }
    switch (i_header.dataType) {
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
