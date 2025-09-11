#ifndef MODBUSHELPER_H
#define MODBUSHELPER_H

#include <QtGlobal>

#include <QVector>

#define REG_MAX 20000

namespace Modbus {

union ModbusValue {
    struct {
        quint16 last;
        quint16 first;
    } in;
    float   outFloat;
    quint32 outUInt;
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

inline float takeFloat(const QVector<quint16> &values) {
    Q_ASSERT(values.size() == 2);
    ModbusValue v;
    v.in.first = values.constFirst();
    v.in.last  = values.constLast();
    return v.outFloat;
}

inline quint32 takeUInt(const QVector<quint16> &values) {
    Q_ASSERT(values.size() == 2);
    ModbusValue v;
    v.in.first = values.constFirst();
    v.in.last  = values.constLast();
    return v.outUInt;
}

}

#endif // MODBUSHELPER_H
