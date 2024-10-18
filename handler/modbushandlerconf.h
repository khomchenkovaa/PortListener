#ifndef MODBUSHANDLERCONF_H
#define MODBUSHANDLERCONF_H

#include "xcsvmodel.h"
#include "dbhandlerconf.h"

#include <QFile>
#include <QTextCodec>

/*
Communication list description

The items meaning of the communication list are as for:
PIN: The Item name of the communication signals. The format is “Signal name.Item name”.
DN: Define the device number of the third-party device. A maximum of 15 devices can be numbered from 1 to 15.
DT: Date type：
        4                             Binary type
        13,14,15,16                   Real type (floating-point type)
        7,8,17,18,19,20,21,22          DWORD type(4-byte integer)
        5,6                          2-byte integer
DL: Date length.
        Binary signal    1
        Analog signal    4
FC: Function code defined in the protocol. Supports 1,2,3,4,5,6,15 and 16.
AD: Start address of the signals.
The address interval between two 2-byte Analog signals is 1
The address interval between two Binary signals is 1
         The address interval between two 4-byte Analog signals is 2
IOT: Input/Output type, Input is 0, output is 1.
PT: Signal type   Binary is 1, Analog is 0.
LP: Define the IP address and the port number of the third-party device.
The port number of the L2COM for the Main I&C system is 502. Russian supply system should configure the same port number.
About the description also refer to the IED document《Communication Specification for SR, NO I&C System and UULS with Third-Party System》


IP: 192.168.1.11, 192.168.2.11
*/
class ModbusCsvConf
{
    enum {
        IdColumn,
        KksColumn,
        DnColumn,
        DtColumn,
        DlColumn,
        FcColumn,
        AdColumn,
        NameColumn,
        DimColumn,
        PointColumn
    };

    struct ModbusCsvConfItem {
        quint32 id;    ///< номер п/п
        QString pin;   ///< Код KKS (The Item name of the communication signals. The format is “Signal name.Item name”)
        quint16 dn;    ///< Define the device number of the third-party device. A maximum of 15 devices can be numbered from 1 to 15.
        quint16 dt;    ///< Data type
        quint16 dl;    ///< Data length
        quint16 fc;    ///< Function code defined in the protocol. Supports 1,2,3,4,5,6,15 and 16
        quint16 ad;    ///< Start address of the signals
        QString name;  ///< Наименование сигнала
        QString dim;   ///< Размерность
        QString point; ///< пункт

        QString dataType() const {
            if (dt == 4) return "Binary type";
            if (dt == 13 || dt == 14 || dt == 15 || dt == 16) return "Real type (floating-point type)";
            if (dt == 7  || dt == 8  || dt == 17 || dt == 18 ||
                dt == 19 || dt == 20 || dt == 21 || dt == 22) return "DWORD type(4-byte integer)";
            if (dt == 5  || dt == 6) return "2-byte integer";
            return QString();
        }
    };

public:
    void load(const QString &fileName) {
        XCsvModel csv;
        csv.setSource(fileName, true, ';', QTextCodec::codecForName("Windows-1251"));
        for (int i = 0; i < csv.rowCount(); ++i) {
            ModbusCsvConfItem item;
            item.id    = csv.data(csv.index(i, IdColumn)).toUInt();
            item.pin   = csv.data(csv.index(i, KksColumn)).toString();
            item.dn    = csv.data(csv.index(i, DnColumn)).toUInt();
            item.dt    = csv.data(csv.index(i, DtColumn)).toUInt();
            item.dl    = csv.data(csv.index(i, DlColumn)).toUInt();
            item.fc    = csv.data(csv.index(i, FcColumn)).toUInt();
            item.ad    = csv.data(csv.index(i, AdColumn)).toUInt();
            item.name  = csv.data(csv.index(i, NameColumn)).toString();
            item.dim   = csv.data(csv.index(i, DimColumn)).toString();
            item.point = csv.data(csv.index(i, PointColumn)).toString();
            items << item;
        }
    }

private:
    QList<ModbusCsvConfItem> items;
};

#endif // MODBUSHANDLERCONF_H
