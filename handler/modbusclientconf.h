#ifndef MODBUSCLIENTCONF_H
#define MODBUSCLIENTCONF_H

#include "modbushelper.h"
#include "xcsvmodel.h"

#include <QFile>
#include <QTextCodec>

/// Signal list description
struct ModbusSigConf
{
    enum {
        KksColumn,
        AddressColumn,
        AvgColumn,
        DataTypeColumn,
        DescrColumn
    };

    struct ModbusSigConfItem {
        QString  pin;    ///< Код KKS (The Item name of the communication signals. The format is “Signal name.Item name”)
        quint16  ad;     ///< Start address of the signals
        quint16  avg;    ///< Average flag
        Modbus::DataType dt;     ///< Data type
        QString  descr;  ///< Описание сигнала

        /**
         * The address interval between two 2-byte Analog signals is 1
         * The address interval between two Binary signals is 1
         * The address interval between two 4-byte Analog signals is 2
         */
        quint16 addressInterval() const {
            switch (dt) {
            case Modbus::BinaryType: return 1;
            case Modbus::RealType:
            case Modbus::DWordType:  return 2;
            case Modbus::IntType:    return 1;
            default: break;
            }
            return 0;
        }

    };

    void load(const QString &fileName) {
        XCsvModel csv;
        csv.setSource(fileName, false, ':');
        for (int i = 0; i < csv.rowCount(); ++i) {
            auto kks = csv.data(csv.index(i, KksColumn)).toString().trimmed();
            if (kks.isEmpty()) continue;
            ModbusSigConfItem item;
            item.pin    = kks;
            item.ad     = csv.data(csv.index(i, AddressColumn)).toUInt();
            item.avg    = csv.data(csv.index(i, AvgColumn)).toUInt();
            item.dt     = dataType(csv.data(csv.index(i, DataTypeColumn)).toString());
            item.descr  = csv.data(csv.index(i, DescrColumn)).toString().trimmed();
            items << item;
        }
    }

    static Modbus::DataType dataType(const QString& value) {
        if (value.startsWith("F", Qt::CaseInsensitive)) {
            return Modbus::RealType;
        }
        if (value.startsWith("I", Qt::CaseInsensitive)) {
            return Modbus::IntType;
        }
        return Modbus::UnkownType;
    }

    QList<ModbusSigConfItem> items;
};

#endif // MODBUSCLIENTCONF_H