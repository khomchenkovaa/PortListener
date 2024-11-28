#ifndef MODBUSHELPER_H
#define MODBUSHELPER_H

#include <QtGlobal>

namespace Modbus {

union ModbusValue {
    struct {
        quint16 last;
        quint16 first;
    } in;
    float   outFloat;
    quint32 outInt;
};

enum DataType {
    UnkownType,
    BinaryType, ///< Binary type
    RealType,   ///< Real type (floating-point type)
    DWordType,  ///< DWORD type(4-byte integer)
    IntType     ///< 2-byte integer
};

}

#endif // MODBUSHELPER_H
