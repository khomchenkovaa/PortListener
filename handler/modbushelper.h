#ifndef MODBUSHELPER_H
#define MODBUSHELPER_H

#include <QtGlobal>

#include <QVector>

#define REG_MAX 20000

namespace Modbus {

union ModbusValue {
    struct {
        quint16 first;
        quint16 last;
    } in;
    float   outFloat;
    quint32 outUInt;
    qint32  outInt;
};

enum DataType {
    UnkownType,
    BinaryType, ///< Binary type
    RealType,   ///< Real type (floating-point type)
    DWordType,  ///< DWORD type(4-byte integer)
    IntType     ///< 2-byte integer
};

enum ByteOrder {
    ABCD,
    DCBA,
    BADC,
    CDAB
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

inline quint16 reverse(quint16 value) {
    return ((value >> 8) & 0xFF) + ((value & 0xFF) << 8);
}

inline void setValues(ModbusValue *v, const QVector<quint16> &values, ByteOrder bo) {
    switch (bo) {
    case ByteOrder::ABCD:
        v->in.first = reverse(values.constFirst());
        v->in.last  = reverse(values.constLast());
        break;
    case ByteOrder::DCBA:
        v->in.first = values.constLast();
        v->in.last  = values.constFirst();
        break;
    case ByteOrder::BADC:
        v->in.first = values.constFirst();
        v->in.last  = values.constLast();
        break;
    case ByteOrder::CDAB:
        v->in.first = reverse(values.constLast());
        v->in.last  = reverse(values.constFirst());
        break;
    }
}

inline QVector<quint16> getValues(ModbusValue *v, ByteOrder bo = ByteOrder::CDAB) {
    QVector<quint16> result;
    switch (bo) {
    case ByteOrder::ABCD:
        result << reverse(v->in.first) << reverse(v->in.last);
        break;
    case ByteOrder::DCBA:
        result << v->in.last << v->in.first;
        break;
    case ByteOrder::BADC:
        result << v->in.first << v->in.last;
        break;
    case ByteOrder::CDAB:
        result << reverse(v->in.last) << reverse(v->in.first);
        break;
    }
    return result;
}

inline float takeFloat(const QVector<quint16> &values, ByteOrder bo = ByteOrder::CDAB) {
    Q_ASSERT(values.size() == 2);
    ModbusValue v;
    setValues(&v, values, bo);
    return v.outFloat;
}

inline quint32 takeUInt(const QVector<quint16> &values, ByteOrder bo = ByteOrder::CDAB) {
    Q_ASSERT(values.size() == 2);
    ModbusValue v;
    setValues(&v, values, bo);
    return v.outUInt;
}

inline qint32 takeInt(const QVector<quint16> &values, ByteOrder bo = ByteOrder::CDAB) {
    Q_ASSERT(values.size() == 2);
    ModbusValue v;
    setValues(&v, values, bo);
    return v.outInt;
}

} // namespace Modbus

#endif // MODBUSHELPER_H
