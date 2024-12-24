#ifndef MODBUSHANDLERCONF_H
#define MODBUSHANDLERCONF_H

#include "modbushelper.h"
#include "xcsvmodel.h"

#include <QFile>
#include <QTextCodec>

/// Communication list description
struct ModbusCsvConf
{
    enum {
        KksColumn = 1,
        DeviceNumberColumn,
        DataTypeColumn,
        DataLengthColumn,
        FunctionCodeColumn,
        AddressColumn,
        IOTypeColumn,
        SignalTypeColumn,
        LPColumn,
        DescrColumn = LPColumn + 2,
        MinColumn,
        MaxColumn,
        UnitsColumn
    };

    struct ModbusCsvConfItem {
        QString pin;    ///< Код KKS (The Item name of the communication signals. The format is “Signal name.Item name”)
        quint16 dn;     ///< Define the device number of the third-party device. A maximum of 15 devices can be numbered from 1 to 15.
        quint16 dt;     ///< Data type
        quint16 dl;     ///< Data length (Binary signal is 1, 2-byte Analog signal is 2, 4-byte Analog signal is 4)
        quint16 fc;     ///< Function code defined in the protocol. Supports 1,2,3,4,5,6,15 and 16
        quint16 ad;     ///< Start address of the signals
        bool    iot;    ///< Input/Output type, Input is 0 (false), output is 1 (true).
        bool    pt;     ///< Signal type, Binary is 1 (true), Analog is 0 (false)
        QString lp;     ///< The IP address and the port number of the third-party device
        QString descr;  ///< Описание сигнала
        QString minVal; ///< Low limit
        QString maxVal; ///< High limit
        QString units;  ///< Unit

        Modbus::DataType dataType() const {
            switch(dt) {
            case 4: return Modbus::BinaryType;
            case 5:
            case 6: return Modbus::IntType;
            case 7:
            case 8: return Modbus::DWordType;
            case 13:
            case 14:
            case 15:
            case 16: return Modbus::RealType;
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22: return Modbus::DWordType;
            default: break;
            }
            return Modbus::UnkownType;
        }

        /**
         * The address interval between two 2-byte Analog signals is 1
         * The address interval between two Binary signals is 1
         * The address interval between two 4-byte Analog signals is 2
         */
        quint16 addressInterval() const {
            switch (dataType()) {
            case Modbus::BinaryType: return 1;
            case Modbus::RealType:
            case Modbus::DWordType:  return 2;
            case Modbus::IntType:    return 1;
            default: break;
            }
            return 0;
        }
    };

    /**
     * @brief load config
     * @param fileName config file name in special csv format
     * @param pt - load binary (true) or analog (false) signals
     */
    void load(const QString &fileName, bool pt = true) {
        XCsvModel csv;
        csv.setSource(fileName, true, ';', QTextCodec::codecForName("Windows-1251"));
        for (int i = 0; i < csv.rowCount(); ++i) {
            auto kks = csv.data(csv.index(i, KksColumn)).toString().trimmed();
            if (kks.isEmpty()) continue;
            ModbusCsvConfItem item;
            item.pin    = kks;
            item.dn     = csv.data(csv.index(i, DeviceNumberColumn)).toUInt();
            item.dt     = csv.data(csv.index(i, DataTypeColumn)).toUInt();
            item.dl     = csv.data(csv.index(i, DataLengthColumn)).toUInt();
            item.fc     = csv.data(csv.index(i, FunctionCodeColumn)).toUInt();
            item.ad     = csv.data(csv.index(i, AddressColumn)).toUInt();
            item.iot    = csv.data(csv.index(i, IOTypeColumn)).toUInt();
            item.pt     = csv.data(csv.index(i, SignalTypeColumn)).toUInt();
            item.lp     = csv.data(csv.index(i, LPColumn)).toString();
            item.descr  = csv.data(csv.index(i, DescrColumn)).toString().trimmed();
            item.minVal = csv.data(csv.index(i, MinColumn)).toString().trimmed();
            item.maxVal = csv.data(csv.index(i, MaxColumn)).toString().trimmed();
            item.units  = csv.data(csv.index(i, UnitsColumn)).toString().trimmed();
            if (item.pt == pt) {
                items << item;
            }
        }
    }

    QList<ModbusCsvConfItem> items;
};

#endif // MODBUSHANDLERCONF_H
