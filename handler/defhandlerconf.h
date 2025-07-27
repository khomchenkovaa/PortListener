#ifndef DEFHANDLERCONF_H
#define DEFHANDLERCONF_H

#include "xutils.h"

namespace Gate {

enum DefDataType {
    defNone = 0,          //!< 0 None (void) data
    defTimeval,           //!< 1 type_timeval
    defRVal,              //!< 2 type_rval
    defIVal,              //!< 3 type_ival
    defSRVal,             //!< 4 type_srval (status ival)
    defSIVal,             //!< 5 type_sival (status rval)
    defLongInt,           //!< 6 int32
    defFloat,             //!< 7 float
    defShortInt,          //!< 8 int16
    defBinShortInt,       //!< 9 int16 with valid 0 and 1
    defDouble,            //!< 10 double
    defErrShortIntVal,    //!< 11 int16 with int16 errcode
    defErrIVal,           //!< 12 type_sival (errcode ival)
    defErrRVal,           //!< 13 type_sival (errcode rval)
    defSByte,             //!< 14 int8
    defBinByte,           //!< 15 (u)int8 with valid 0 and 1
    defTimespec,          //!< 16 type_timespec
    defDoubleDaySecsUTC,  //!< 17 double - seconds from midnight (UTC)
    defDoubleDaySecsLocal,//!< 18 double - seconds from midnight (Local time)
    defLongUInt,          //!< 19 uint32
    defTimespec64,        //!< 20 type_timespec64 (8 bytes+8 bytes)
    defInt64,             //!< 21 int64
    defUInt64,            //!< 22 uint64
    defCount              //!< fake element for count enum's members
};

inline QString defDataTypeName(int idx) {
    switch (idx) {
    case DefDataType::defTimeval:            return "type_timeval";
    case DefDataType::defRVal:               return "type_rval";
    case DefDataType::defIVal:               return "type_ival";
    case DefDataType::defSRVal:              return "type_srval";
    case DefDataType::defSIVal:              return "type_sival";
    case DefDataType::defLongInt:            return "type_longint";
    case DefDataType::defFloat:              return "type_float";
    case DefDataType::defShortInt:           return "type_shortint";
    case DefDataType::defBinShortInt:        return "type_binshortint";
    case DefDataType::defDouble:             return "type_double";
    case DefDataType::defErrShortIntVal:     return "type_errshortintval";
    case DefDataType::defErrIVal:            return "type_errival";
    case DefDataType::defErrRVal:            return "type_errrval";
    case DefDataType::defSByte:              return "type_sbyte";
    case DefDataType::defBinByte:            return "type_binbyte";
    case DefDataType::defTimespec:           return "type_timespec";
    case DefDataType::defDoubleDaySecsUTC:   return "type_doubledaysecsuts";
    case DefDataType::defDoubleDaySecsLocal: return "type_doubledaysecslocal";
    case DefDataType::defLongUInt:           return "type_longuint";
    case DefDataType::defTimespec64:         return "type_timespec64";
    case DefDataType::defInt64:              return "type_int64";
    case DefDataType::defUInt64:             return "type_uint64";
    default: break;
    }
    return QString();
}

inline DefDataType defDataType(const QString &typeName) {
    for (int i=DefDataType::defTimeval; i<DefDataType::defCount; ++i) {
        if (typeName.compare(defDataTypeName(i), Qt::CaseInsensitive) == 0) {
            return static_cast<DefDataType>(i);
        }
    }
    return DefDataType::defNone;
}

class DefConfig
{
    enum {
        NameColumn,
        TypeColumn,
        OffsetColumn,
        LayersColumn,
        RowsColumn,
        ColumnsColumn
    };

    struct DefConfigItem {
        QString     name;        ///< код параметра (kks, rtm и iid)
        DefDataType type;        ///< тип
        size_t      offset  = 0; ///< Смещение в байтах от начала области
        size_t      layers  = 1; ///< Слои - позволяет задать массивы, но используются только 1 (один слой)
        size_t      rows    = 1; ///< Ряды - позволяет задать массивы, но используются только 1 (один ряд)
        size_t      columns = 1; ///< Колонки - позволяет задать массивы, но используются только 1 (одна колонка)
    };

    struct DefConfItemPrivate {
        size_t areaSize        = 0;
        size_t timePointOffset = 0;
        size_t timePointSize   = 0;
        QList<DefConfigItem> items;
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
                size_t countOfFields = line.toUInt();
                Q_UNUSED(countOfFields)
                state = 1;
                continue;
            }
            if (state == 1) {
                d.areaSize = line.toInt();
                state = 2;
                continue;
            }
            const auto values = Utils::parseCsvRow(line, ',');
            if (values.size() > ColumnsColumn) {
            DefConfigItem item;
                item.name    = values.at(NameColumn);
                item.type    = defDataType(values.at(TypeColumn));
                item.offset  = values.at(OffsetColumn).toUInt();
                item.layers  = values.at(LayersColumn).toUInt();
                item.rows    = values.at(RowsColumn).toUInt();
                item.columns = values.at(ColumnsColumn).toUInt();
                d.items << item;
            }
        }

        file.close();
    }

    int areaSize() const {
        return d.areaSize;
    }

    int itemsCount() const {
        return d.items.size();
    }

    QString name(int idx) const {
        if (idx < d.items.size()) return d.items.at(idx).name;
        return QString();
    }

    QString type(int idx) const {
        return idx < d.items.size() ? defDataTypeName(d.items.at(idx).type) : QString();
    }

    DefDataType typeId(int idx) const {
        return idx < d.items.size() ? d.items.at(idx).type : DefDataType::defNone;
    };

    quint64 offset(int idx) const {
        return idx < d.items.size() ? d.items.at(idx).offset : 0;
    }

private:
    DefConfItemPrivate d;
};

} // namespace Gate

#endif // DEFHANDLERCONF_H
