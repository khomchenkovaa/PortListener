#include "depstructs.h"
#include "mathutils.h"
#include "timeutils.h"

#include <QDataStream>
#include <QDebug>

/********************************************************/

QString DEPHeader::toStr() const
{
    return QString("DEPHeader: module=%1  len=%2  cs=%3").arg(module).arg(len).arg(Utils::uint32ToBAStr(cs, true));
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
    stream >> len >> cs;
}

/********************************************************/

void DEPHeader::toStream(QDataStream &stream, bool without_cs)
{
    stream << module;
    for (int i=0; i<20; i++) stream << qint8(reserve[i]);
    stream << len;
    if (without_cs) return;
    stream << cs;
}

/********************************************************/

QString DEPInternalHeader::toStr() const
{
    return QString("DEPInternalHeader: header_len=%1  VERSION(%2)  metod=%3  data_type=%4  common_time=%5  param_time=%6  start_index=%7  param_count=%8").
            arg(header_len).arg(Utils::uint32ToBAStr(ver, true)).arg(metod).arg(data_type).
            arg(common_time).arg(param_time).arg(start_index).arg(param_count);
}

/********************************************************/

void DEPInternalHeader::fromDataStream(QDataStream &stream)
{
    stream >> header_len >> ver >> metod >> data_type >> common_time >> param_time >> start_index >> param_count;
}

/********************************************************/

void DEPInternalHeader::toStream(QDataStream &stream)
{
    stream << header_len << ver << metod << data_type << common_time << param_time << start_index << param_count;
}

/********************************************************/

void DEPInternalHeader::prepare(int p_type, int p_count, quint32 pos)
{
    w32_time_us t;

    //prepare internal header fields
    header_len = DEPInternalHeader::byteSize() + t.size();
    ver = quint32(DEP_PARAM_PACK_VERSION);
    metod = ptIndividual;
    data_type = p_type;
    common_time = tptUTmsecUTC;
    param_time = tptUTmsecUTC;
    start_index = pos;
    param_count = p_count;
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
