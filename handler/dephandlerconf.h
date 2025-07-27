#ifndef DEPHANDLERCONF_H
#define DEPHANDLERCONF_H

#include "xcsvmodel.h"
#include "xutils.h"

#include <QTextCodec>

namespace Gate {

enum DPDataType {
    NONE               =  0, ///< Несуществующий тип, но теоретически может быть установлен в результате ошибки
    BYTE               =  1, ///< Байт (unsigned char)
    SBYTE              =  2, ///< Знаковый байт (signed char)
    WORD               =  3, ///< Слово (unsigned short/quint16)
    SWORD              =  4, ///< Знаковое слово (signed short/qint16)
    DWORD              =  5, ///< Двойное слово (unsigned int/quint32)
    SDWORD             =  6, ///< Знаковое двойное слово (signed int/qint32)
    QWORD              =  7, ///< Четверное слово (unsigned long long/quint64)
    SQWORD             =  8, ///< Знаковое четверное слово (signed long long/qint64)
    FLOAT              =  9, ///< float (4 байтовое значение с плавающей запятой IEEE-754)
    DOUBLE             = 10, ///< double (8 байтовое значение с плавающей запятой IEEE-754)
    LongDouble         = 11, ///< long double (10 байтовое значение с плавающей запятой IEEE-754) (Не реализован)
    BIT                = 12, ///< Бит (в байте) (Не используется)
    BitField           = 13, ///< Битовое поле (беззнаковая последовательность бит в одном и более байте) (Не реализован)
    SBitField          = 14, ///< Знаковое битовое поле (знаковая последовательность бит в одном и более байте) (Не реализован)
    BOOL               = 15, ///< Логическое (bool) однобайтовое значение (0 - ложь, не 0 - истина)
    ASCIIZ             = 16, ///< ASCIIz (оканчивающаяся на 0 строка) (Не реализован)
    SM2MCODE           = 17, ///< код "СМ-2М" - 2-х байтовое значение с фиксированной точкой особого формата (Не реализовано)
    FLOATVALID         = 18, ///< float с достоверностью int (qint32) (8-ми байтовая структура)
    SDWORDVALID        = 19, ///< int (qint32) с достоверностью int (qint32) (8-ми байтовая структура)
    TIME_T32           = 20, ///< Unix time_t (uint32) секунды с 01/01/1970 UTC
    FLOATSTATE         = 21, ///< float со статусом int (qint32) (8-ми байтовая структура)
    SDWORDSTATE        = 22, ///< int (qint32) со статусом int (qint32) (8-ми байтовая структура)
    TIMEVAL            = 23, ///< timeval: time_t + мкс (8-ми байтовая структура)
    BLOBQSTRING        = 24, ///< DWORD (или WORD) смещение в дополнительном буфере (не реализовано)
    KIVALID            = 25, ///< DWORD с КИ достоверностью 0 - Н/В, 1 - OK, -1(0xFFFFFFFF) - отключено (или не инициировано)
    KISTATE            = 26, ///< DWORD с состоянием КИ 0 - OK, 7 - Н/В, 1..6 - разные превышения уставок
    ERRCODEDWORD       = 27, ///< DWORD с 0 - OK, иначе - код ошибки
    VNIIEMDWORD        = 28, ///< DWORD с дискретом ВНИИЭМ: 0 - Откл, 1 - Вкл, -1(0xFFFFFFFF) - Н/В (Не используется)
    VNIIEMBYTE         = 29, ///< BYTE с дискретом ВНИИЭМ: 0 - Откл, 1 - Вкл, 255 - Н/В (Не используется)
    FLOATERRCODE       = 30, ///< float с int (qint32) кодом ошибки (0 - OK) (8-ми байтовая структура)
    SDWORDERRCODE      = 31, ///< int (qint32)  с int (qint32) кодом ошибки (0 - OK) (8-ми байтовая структура)
    WORDDISCRETE       = 32, ///< Word (quint16) дискрет 0 - Откл, 1 - Вкл, Иначе (0xFFFF) - Н/В
    DTSFLOAT           = 33, ///< float с побитным качеством (ППД) (8-ми байтовая структура) - соответствует AData
    DTSSDWORD          = 34, ///< int (qint32) с побитным качеством (ППД) (8-ми байтовая структура) - соответствует IData
    DTSSDWORDDISCRETE  = 35, ///< int (qint3) с побитным качеством (ППД) (8-ми байтовая структура) - как предыдущий, но соответствует BData
    SWORDERRCODE       = 36, ///< short (qint16) с short (int16) кодом ошибки (0 - OK) (4-ми байтовая структура)
    ERRCODEWORD        = 37, ///< WORD (quint16/qint16 - нужно уточнение) код ошибки: 1 - OK, иначе код
    TIMESPEC           = 38, ///< timeval: time_t + наносекунды (8-ми байтовая структура)
    TIMEWITHMSECS      = 39, ///< time_t + миллисекунды (8-ми байтовая структура)
    DOUBLEDAYSECSUTC   = 40, ///< double секунды с полуночи (UTC)
    DOUBLEDAYSECSLOCAL = 41, ///< double секунды с полуночи (местное время)
    DOUBLEVALID        = 42, ///< double с int достоверностью (12-ти байтовая структура) (Не используется)
    TIMESPEC64         = 43  ///< timespec для 64-ти битовых time_t and nsec: time_t (8 байт) + наносекунды (8 байт) (16-ти байтовая структура)
};

inline QString depDataTypeName(int idx) {
    switch (idx) {
    case DPDataType::BYTE:               return "BYTE";
    case DPDataType::SBYTE:              return "SBYTE";
    case DPDataType::WORD:               return "WORD";
    case DPDataType::SWORD:              return "SWORD";
    case DPDataType::DWORD:              return "DWORD";
    case DPDataType::SDWORD:             return "SDWORD";
    case DPDataType::QWORD:              return "QWORD";
    case DPDataType::SQWORD:             return "SQWORD";
    case DPDataType::FLOAT:              return "FLOAT";
    case DPDataType::DOUBLE:             return "DOUBLE";
    case DPDataType::LongDouble:         return "(LongDouble)";
    case DPDataType::BIT:                return "BIT";
    case DPDataType::BitField:           return "(BitField)";
    case DPDataType::SBitField:          return "(SBitField)";
    case DPDataType::BOOL:               return "BOOL";
    case DPDataType::ASCIIZ:             return "ASCIIZ";
    case DPDataType::SM2MCODE:           return "SM2MCODE";
    case DPDataType::FLOATVALID:         return "FLOATVALID";
    case DPDataType::SDWORDVALID:        return "SDWORDVALID";
    case DPDataType::TIME_T32:           return "TIME_T32";
    case DPDataType::FLOATSTATE:         return "FLOATSTATE";
    case DPDataType::SDWORDSTATE:        return "SDWORDSTATE";
    case DPDataType::TIMEVAL:            return "TIMEVAL";
    case DPDataType::BLOBQSTRING:        return "BLOBQSTRING";
    case DPDataType::KIVALID:            return "KIVALID";
    case DPDataType::KISTATE:            return "KISTATE";
    case DPDataType::ERRCODEDWORD:       return "ERRCODEDWORD";
    case DPDataType::VNIIEMDWORD:        return "VNIIEMDWORD";
    case DPDataType::VNIIEMBYTE:         return "VNIIEMBYTE";
    case DPDataType::FLOATERRCODE:       return "FLOATERRCODE";
    case DPDataType::SDWORDERRCODE:      return "SDWORDERRCODE";
    case DPDataType::WORDDISCRETE:       return "WORDDISCRETE";
    case DPDataType::DTSFLOAT:           return "DTSFLOAT";
    case DPDataType::DTSSDWORD:          return "DTSSDWORD";
    case DPDataType::DTSSDWORDDISCRETE:  return "DTSSDWORDDISCRETE";
    case DPDataType::SWORDERRCODE:       return "SWORDERRCODE";
    case DPDataType::ERRCODEWORD:        return "ERRCODEWORD";
    case DPDataType::TIMESPEC:           return "TIMESPEC";
    case DPDataType::TIMEWITHMSECS:      return "TIMEWITHMSECS";
    case DPDataType::DOUBLEDAYSECSUTC:   return "DOUBLEDAYSECSUTC";
    case DPDataType::DOUBLEDAYSECSLOCAL: return "DOUBLEDAYSECSLOCAL";
    case DPDataType::DOUBLEVALID:        return "DOUBLEVALID";
    case DPDataType::TIMESPEC64:         return "TIMESPEC64";
    }
    return QString();
}

inline DPDataType depDataTypeFromString(const QString &typeName) {
    for (int i=DPDataType::BYTE; i<=DPDataType::TIMESPEC64; ++i) {
        if (typeName.compare(depDataTypeName(i), Qt::CaseInsensitive) == 0) {
            return static_cast<DPDataType>(i);
        }
    }
    return DPDataType::NONE;
}

inline DPDataType depTypeByString(const QString &str) {
    if (str.compare("A", Qt::CaseInsensitive) == 0) return DPDataType::DTSFLOAT;
    if (str.compare("R", Qt::CaseInsensitive) == 0) return DPDataType::DTSFLOAT;
    if (str.compare("D", Qt::CaseInsensitive) == 0) return DPDataType::DTSSDWORDDISCRETE;
    if (str.compare("B", Qt::CaseInsensitive) == 0) return DPDataType::DTSSDWORDDISCRETE;
    if (str.compare("I", Qt::CaseInsensitive) == 0) return DPDataType::DTSSDWORD;
    if (str.compare("N", Qt::CaseInsensitive) == 0) return DPDataType::DTSSDWORD;
    return static_cast<DPDataType>(depDataTypeFromString(str));
}


struct CsvConfig
{
    struct CsvProperties {
        QString typeValue = "NONE";  ///< datatype name
        int typeColumn    = -1;      ///< datatype column index
        int indexColumn   = -1;      ///< id column index: >=0 (otherwise error)
        int extIdColumn   = -1;      ///< extid column index: >=0 (otherwise not used)
        int iidColumn     = -1;      ///< iid column index: >=0 (otherwise not used)
        int kksColumn     = -1;      ///< kks column index: >=0 (otherwise not used)
        int rtmColumn     = -1;      ///< rtm column index: >=0 (otherwise not used)

        int maxIndex() const {
            int result = qMax(typeColumn, indexColumn);
            result = qMax(result, extIdColumn);
            result = qMax(result, iidColumn);
            result = qMax(result, kksColumn);
            result = qMax(result, rtmColumn);
            return result;
        }
    };

    struct CsvConfigItem {
        DPDataType type;  ///< data type
        quint32    index; ///< номер
        QString    kks;   ///< Код KKS
        QString    iid;   ///< Interal ID
    };

    QList<CsvConfigItem> items;
    CsvProperties csv;

    int load(const QString &fileName, QChar separator = ',', QTextCodec *codec = Q_NULLPTR) {
        XCsvModel csvModel;
        csvModel.setSource(fileName, true, separator, codec);
        if (csvModel.columnCount() <= csv.maxIndex()) {
            return -1; // lines too short
        }
        DPDataType defaultType = depTypeByString(csv.typeValue);
        if (defaultType == DPDataType::NONE && csv.typeColumn == -1) {
            return -2; // type value is not defined
        }
        if (csv.indexColumn == -1) {
            return -3; // index is not defined
        }
        if (csv.kksColumn == -1 && csv.iidColumn == -1) {
            return -4; // kks or iid is not defined
        }
        for (int i = 0; i < csvModel.rowCount(); ++i) {
            CsvConfigItem item;
            if (csv.typeColumn != -1) {
                QString typeStr = csvModel.data(csvModel.index(i, csv.typeColumn)).toString();
                item.type = depTypeByString(typeStr);
            } else {
                item.type = defaultType;
            }
            item.index = csvModel.data(csvModel.index(i, csv.indexColumn)).toUInt();
            if (csv.kksColumn != -1) {
                item.kks = csvModel.data(csvModel.index(i, csv.kksColumn)).toString();
            }
            if (csv.iidColumn != -1) {
                item.iid = csvModel.data(csvModel.index(i, csv.iidColumn)).toString();
            }
            items << item;
        }
        return items.size();
    }
};

} // namespace Gate

#endif // DEPHANDLERCONF_H
