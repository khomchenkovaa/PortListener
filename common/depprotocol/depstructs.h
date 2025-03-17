#ifndef DEP_STRUCTS_H
#define DEP_STRUCTS_H

//структуры DEP протокола, служат для формирования/разбора DEP пакетов.
//в одном пакете могут быть только данные одного типа (из множества DEPDataType)
//если передаваемых типов параметров несколько, то на каждый тип необходимо создавать свой пакет.
//тип данных параметров лежащих в пакете указывается в поле DEPInternalHeader.data_type

//структура полного пакета:
//1. внешний заголовок DEPHeader (32 bytes)
//2. DEP данные (внутренний пакет)
//3. КС всего пакета  (4 bytes)

// структура внутреннего пакета:
// 1. внутренний заголовок DEPInternalHeader
// 2. локальное время компьюьера, (переменная w32_time_us)
// 3. N-секций параметров по 20 байт

//пример одной секции типа dpdtFloatValid:
//    quint32 pack_index;  - служебный индекс, указывается в конфиге, для совместимости с принимающей стороной
//    QDateTime dt;  - временная метка
//    float value; - непосредственно само значение параметра
//    qint32 validity; - достоверность  значения параметра

#include "depenums.h"

#include <QDateTime>

class QDataStream;

/**
 * @brief dep protocol header, 32 bytes
 * внешний заголовок окончательного пакета,
 * внутренний пакет(созданный по правилам DEP-протокола) как-бы обернут внешним заголовком и КС всего пакета
 */
struct DEPHeader
{
    quint32 module = depsParamPacket;  ///< елемент множества DEPSignatures
    char    reserve[20];               ///< пустая область (резерв)
    quint32 len = 0;                   ///< размер только внутренних данных, т.е. без учета размера этого внешнего заголовка и КС, которая находится в конце полного пакета
    quint32 cs  = 0;                   ///< header checksum

    DEPHeader() {
        memset(reserve, 0, sizeof(reserve));
    }

    DEPHeader(quint32 m) {
        module = m;
        memset(reserve, 0, sizeof(reserve));
    }

    bool isModuleValid() const {
        return (module == depsParamPacket || module == depsStubPacket);
    }

    bool isChecksumOK() const {
        const auto buff = reinterpret_cast<const char *>(this);
        quint32 sum = qChecksum(buff, DEPHeader::size() - sizeof(quint32));
        return (cs == sum);
    }

    QString toString() const;

    /// @deprecated
    void fromDataStream(QDataStream& stream);

    /// @deprecated
    void toStream(QDataStream& stream, bool without_cs = false);

    /// размер всего полного пакета
    quint32 packetSize() const {
        return DEPHeader::size() + len + sizeof(quint32);
    }

    /// размер всего полного пакета без КС в конце него
    quint32 packetSizeWithoutCs() const {
        return DEPHeader::size() + len;
    }

    /// размер заголовка
    static int size() {
        return (sizeof(module) + sizeof(reserve) + sizeof(len) + sizeof(cs));
    }
};

/// dep protocol internal header
struct DEPInternalHeader
{
    quint32 headerSize = 0; ///< Размер этого заголовка (+опциональные данные до начала параметров)
    quint32 version    = 0; ///< Версия (Сейчас 0x10000 = "0.1.0.0")
    quint32 packType   = 0; ///< способ паковки DEPPackingType: 1 - Индивидуально, 2 - Массив (пока реализовано только для варианта - 1)
    quint32 dataType   = 0; ///< данные: id типа данных  (елемент множества DEPDataType), пока реализовано только для типов: dpdtFloatValid, dpdtSDWordValid
    quint32 commonTime = 0; ///< тип общей метки времени: 0 - нет, или значение из множества DEPTimePoint
    quint32 paramTime  = 0; ///< тип индивидуальной метки времени параметров: аналогично предыдущему
    quint32 startIndex = 0; ///< Начальные индекс для паковки последовательным массивом (иначе 0)
    quint32 paramCount = 0; ///< Число параметров

    QString toString() const;
    void fromDataStream(QDataStream& stream);
    void toStream(QDataStream& stream);

    /// инициализировать заголовок перед отправкой пакета
    void prepare(int p_type, int p_count, quint32 pos = 0);

    static int size() {
        return (sizeof(headerSize) + sizeof(version)   + sizeof(packType)   + sizeof(dataType) +
                sizeof(commonTime) + sizeof(paramTime) + sizeof(startIndex) + sizeof(paramCount));
    }
};

/// DEPFloat parameter section
struct DEPFloatValidRecord
{
    enum { REC_SIZE = 20 };

    quint32   pack_index = 0;
    QDateTime dt;
    float     value      = -1;
    qint32    validity   = 0;

    QString toString() const {
        return QString("DEPFloatValidRecord: pos=%1 value=%2 validity=%3")
                .arg(pack_index).arg(value).arg(validity);
    }

    void fromDataStream(QDataStream&);
};

/// DEPSDWord parameter section
struct DEPSDWordValidRecord
{
    enum { REC_SIZE = 20 };

    quint32   pack_index = 0;
    QDateTime dt;
    qint32    value      = -1;
    qint32    validity   = 0;

    QString toString() const {
        return QString("DEPSDWordValidRecord: pos=%1 value=%2 validity=%3")
                .arg(pack_index).arg(value).arg(validity);
    }

    void fromDataStream(QDataStream&);
};

#endif //DEP_STRUCTS_H
