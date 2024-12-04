#ifndef MODBUSHELPER_H
#define MODBUSHELPER_H

#include <QtGlobal>

#define REG_MAX 2000

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

/**
 * The address interval between two 2-byte Analog signals is 1
 * The address interval between two Binary signals is 1
 * The address interval between two 4-byte Analog signals is 2
 */
inline quint16 dataTypeSizeOf(DataType dt, quint16 valDefault = 1) {
    switch (dt) {
    case Modbus::BinaryType: return 1;
    case Modbus::RealType:
    case Modbus::DWordType:  return 2;
    case Modbus::IntType:    return 1;
    default: break;
    }
    return valDefault;
}

}

#endif // MODBUSHELPER_H
