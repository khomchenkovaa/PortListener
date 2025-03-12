#ifndef DEPHANDLERCONF_H
#define DEPHANDLERCONF_H

#include "xcsvmodel.h"
#include "xutils.h"

#include <QFile>
#include <QTextCodec>

namespace Gate {

enum TypeValue {
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

inline int typeValueFromString(const QString &typeName) {
    static const QStringList names = QString(
                "NONE,BYTE,SBYTE,WORD,SWORD,DWORD,SDWORD,QWORD,SQWORD,FLOAT,"
                "DOUBLE,(LongDouble),BIT,(BitField),(SBitField),BOOL,ASCIIZ,SM2MCODE,FLOATVALID,SDWORDVALID,"
                "TIME_T32,FLOATSTATE,SDWORDSTATE,TIMEVAL,BLOBQSTRING,KIVALID,KISTATE,ERRCODEDWORD,VNIIEMDWORD,VNIIEMBYTE,"
                "FLOATERRCODE,SDWORDERRCODE,WORDDISCRETE,DTSFLOAT,DTSSDWORD,DTSSDWORDDISCRETE,SWORDERRCODE,ERRCODEWORD,TIMESPEC,TIMEWITHMSECS,"
                "DOUBLEDAYSECSUTC,DOUBLEDAYSECSLOCAL,DOUBLEVALID,TIMESPEC64"
                ).split(',');
    return names.indexOf(typeName);
}

class CsvConf
{
    enum {
        NumColumn,
        TypeColumn,
        NameColumn,
        DimColumn,
        KksColumn,
        PointColumn
    };

    struct CsvConfItem {
        quint32 num;   ///< номер
        QString type;  ///< тип
        QString name;  ///< Наименование сигнала
        QString dim;   ///< Размерность
        QString kks;   ///< Код KKS
        QString point; ///< пункт
    };

public:
    void load(const QString &fileName) {
        XCsvModel csv;
        csv.setSource(fileName, true, ',', QTextCodec::codecForName("Windows-1251"));
        for (int i = 0; i < csv.rowCount(); ++i) {
            CsvConfItem item;
            item.num   = csv.data(csv.index(i, NumColumn)).toUInt();
            item.type  = csv.data(csv.index(i, TypeColumn)).toString();
            item.name  = csv.data(csv.index(i, NameColumn)).toString();
            item.dim   = csv.data(csv.index(i, DimColumn)).toString();
            item.kks   = csv.data(csv.index(i, KksColumn)).toString();
            item.point = csv.data(csv.index(i, PointColumn)).toString();
            items << item;
        }
    }

private:
    QList<CsvConfItem> items;
};

class DefConf {

    enum {
        NameColumn,
        TypeColumn,
        OffsetColumn,
        LayersColumn,
        RowsColumn,
        ColumnsColumn
    };

    struct DefConfItem {
        QString name;        ///< код параметра (kks, rtm и iid)
        QString type;        ///< тип
        quint64 offset  = 0; ///< Смещение в байтах от начала области
        quint16 layers  = 1; ///< Слои - позволяет задать массивы, но используются только 1 (один слой)
        quint16 rows    = 1; ///< Ряды - позволяет задать массивы, но используются только 1 (один ряд)
        quint16 columns = 1; ///< Колонки - позволяет задать массивы, но используются только 1 (одна колонка)
    };

    struct DefConfItemPrivate {
        quint32 fields = 0;  ///< Число полей (параметров), игнорируется, по умолчанию - 0
        int     size   = 4;  ///< Размер области памяти в байтах (должно вмещать все параметры, но может быть и с запасом)
        QList<DefConfItem> items;
    };

public:
    void load(const QString &fileName) {
        quint8 state = 0;
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        while (!file.atEnd()) {
            QString line = file.readLine().trimmed();
            if (line.isEmpty()) continue;
            if (line.startsWith('#')) continue;
            if (state == 0) {
                d.fields = line.toUInt();
                state = 1;
                continue;
            }
            if (state == 1) {
                d.size = line.toInt();
                state = 2;
                continue;
            }
            const auto values = Utils::parseCsvRow(line, ',');
            DefConfItem item;
            if (values.size() > NameColumn)    item.name    = values.at(NameColumn);
            if (values.size() > TypeColumn)    item.type    = values.at(TypeColumn);
            if (values.size() > OffsetColumn)  item.offset  = values.at(OffsetColumn).toUInt();
            if (values.size() > LayersColumn)  item.layers  = values.at(LayersColumn).toUInt();
            if (values.size() > RowsColumn)    item.rows    = values.at(RowsColumn).toUInt();
            if (values.size() > ColumnsColumn) item.columns = values.at(ColumnsColumn).toUInt();
            d.items << item;
        }

        file.close();
    }

    int size() const {
        return d.size;
    }

    int fields() const {
        return d.items.size();
    }

    QString name(int idx) const {
        if (idx < d.items.size()) return d.items.at(idx).name;
        return QString();
    }

    QString type(int idx) const {
        if (idx < d.items.size()) return d.items.at(idx).type;
        return QString();
    }

    int typeId(int idx) const {
        static const QStringList typeVals = QStringList()
                << "type_timeval"            //  1 - TIMEVAL
                << "type_rval"               //  2 - FLOATVALID
                << "type_ival"               //  3 - SDWORDVALID
                << "type_srval"              //  4 - FLOATSTATE
                << "type_sival"              //  5 - SDWORDSTATE
                << "type_longint"            //  6 - SDWORD (int32)
                << "type_float"              //  7 - FLOAT (float)
                << "type_shortint"           //  8 - SWORD (int16)
                << "type_binshortint"        //  9 - WORDDISCRETE (int16 0/1/-1)
                << "type_double"             // 10 - DOUBLE (double)
                << "type_errshortintval"     // 11 - SWORDERRCODE (int16+int16)
                << "type_errival"            // 12 - SDWORDERRCODE (int32+int32)
                << "type_errrval"            // 13 - FLOATERRCODE (float+int32)
                << "type_sbyte"              // 14 - SBYTE (int8)
                << "type_binbyte"            // 15 - BOOL ((u)int8 с 0 and 1(не 0))
                << "type_timespec"           // 16 - TIMESPEC (8-ми байтная структура)
                << "type_doubledaysecsuts"   // 17 - DOUBLEDAYSECSUTC (double)
                << "type_doubledaysecslocal" // 18 - DOUBLEDAYSECSLOCAL (double)
                << "type_longuint"           // 19 - DWORD (uint32)
                << "type_timespec64"         // 20 - TIMESPEC64 (8 байт+8 байт)
                << "type_int64"              // 21 - SQWORD (int64)
                << "type_uint64";            // 22 - QWORD (uint64)
        return typeVals.indexOf(type(idx)) + 1; // not found (-1) is mapped to NONE (0)
    };

    quint64 offset(int idx) const {
        if (idx < d.items.size()) return d.items.at(idx).offset;
        return 0;
    }


private:
    DefConfItemPrivate d;
};

} // namespace Gate

#endif // DEPHANDLERCONF_H
