#ifndef DEP_ENUMS_H
#define DEP_ENUMS_H

#include <QString>

enum {
    DEP_PARAM_PACK_VERSION = 0x10000,
    VIEW_PARAM_SIZE = 8
};

/// dep header signature
enum DEPSignatures
{
    depsStubPacket  = 0,          ///< QDEStubPackProcessor() may be used for keepAlive packets
    depsParamPacket = 0x01000001  ///< QDEParamPackProcessor
};

/// тип данных параметров
enum DEPDataType
{
    dpdtNone = 0,        ///< 0 None (void) data
    dpdtByte,            ///< 1 Byte (unsigned char) data
    dpdtSByte,           ///< 2 Signed Byte (signed char) data
    dpdtWord,            ///< 3 Word (unsigned short) data
    dpdtSWord,           ///< 4 Signed Word (signed short) data
    dpdtDWord,           ///< 5 DWord (unsigned int) data
    dpdtSDWord,          ///< 6 Signed DWord (signed int) data
    dpdtQWord,           ///< 7 QWord (unsigned long long) data
    dpdtSQWord,          ///< 8 Signed QWord (signed long long) data
    dpdtFloat,           ///< 9 float data
    dpdtDouble,          ///< 10 double data
    dpdtFloatValid = 18, ///< 18 Float with int validity (8 byte struct)
    dpdtSDWordValid,     ///< 19 int (sdword) with int validity (8 byte struct)
    dpdtTime_t32         ///< 20 Unix time_t (uint32) - secs since 1 Jan 1970 UTC
};

/// тип временной метки
enum DEPTimePoint
{
    tptNone = 0,      ///< 0 No timepoint
    tptRaw,           ///< 1 Raw bytearray (only look on change)
    tptUnixTime,      ///< 2 Unix time: second from 1970-01-01 00:00:00 UTC
    tptLocalUnixTime, ///< 3 "Unix time" second from 1970-01-01 00:00:00 Local
    tptKILocal,       ///< 4 KI "timeval": second from 1970-01-01 00:00:00  Local + usec (microseconds)
    tptKIUTC,         ///< 5 UTC KI "timeval": second from 1970-01-01 00:00:00 UTC + usec (microseconds)
    tptUTmsecUTC,     ///< 6 Unix time: (UTC) seconds + msec (milliseconds)
    tptUTmsecLocal,   ///< 7 Local "Unix time" seconds + msec (milliseconds)
    tptUTdoubleUTC,   ///< 8 Unix time: (UTC) seconds with fractional
    tptUTdoubleLocal, ///< 9 Local "Unix time" seconds  with fractional
    tptTimeSpec,      ///< 10 timespec (32 bits) second from 1970-01-01 00:00:00 UTC + nsec (nanoseconds) (int32+int32)
    tptTimeSpec64,    ///< 11 timespec (64 bits) second from 1970-01-01 00:00:00 UTC + nsec (nanoseconds) (int64+int64)
};

#endif //DEP_ENUMS_H
