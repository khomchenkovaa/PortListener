#ifndef DEP_STRUCTS_H
#define DEP_STRUCTS_H

/*
структуры DEP протокола, служат для формирования/разбора DEP пакетов.
в одном пакете могут быть только данные одного типа (из множества DEPDataType)
если передаваемых типов параметров несколько, то на каждый тип необходимо создавать свой пакет.
тип данных параметров лежащих в пакете указывается в поле DEPDataHeader.data_type

структура полного пакета:
1. внешний заголовок DEPHeader (32 bytes)
2. DEP данные (внутренний пакет)
3. КС всего пакета  (4 bytes)

структура внутреннего пакета:
1. внутренний заголовок DEPDataHeader
2. локальное время компьюьера, (переменная w32_time_us)
3. N-секций параметров по 20 байт

пример одной секции типа dpdtFloatValid:
   quint32 pack_index;  - служебный индекс, указывается в конфиге, для совместимости с принимающей стороной
   QDateTime dt;  - временная метка
   float value; - непосредственно само значение параметра
   qint32 validity; - достоверность  значения параметра
*/

#include "depenums.h"

#include <QVariant>
#include <QDateTime>

class QDataStream;

/**
 * @brief dep protocol header, 32 bytes
 * внешний заголовок окончательного пакета,
 * внутренний пакет(созданный по правилам DEP-протокола) как-бы обернут внешним заголовком и КС всего пакета
 */
struct DEPHeader
{
    enum { REC_SIZE = 32 };

    quint32 module = depsParamPacket;  ///< елемент множества DEPSignatures
    char    reserve[20];               ///< пустая область (резерв)
    quint32 bodySize       = 0;        ///< размер только внутренних данных, т.е. без учета размера этого внешнего заголовка и КС, которая находится в конце полного пакета
    quint32 headerChecksum = 0;        ///< header checksum

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

    QString toString() const;

    void fromDataStream(QDataStream& stream);

    void toStream(QDataStream& stream, bool without_cs = false);

    /// размер всего полного пакета
    quint32 packetSize() const {
        return packetSizeWithoutCs() + sizeof(quint32);
    }

    /// размер всего полного пакета без КС в конце него
    quint32 packetSizeWithoutCs() const {
        return DEPHeader::REC_SIZE + bodySize;
    }
};

/// dep protocol internal data header
struct DEPDataHeader
{
    enum { REC_SIZE = 32 };

    /// тип паковки данных в пакете
    enum PackingType {
        ptNone = 0,   ///< 0 - No data (should no be used)
        ptIndividual, ///< 1 - Individual (each param with ID)
        ptArray,      ///< 2 - Array (range from N to N+C-1)
        ptCount,      ///< 3 - Values count (for service purpose)
    };

    quint32 headerSize = 40;                           ///< Размер заголовка. REC_SIZE + опциональные данные sizeof(w32_time_us)
    quint32 version    = DEP_PARAM_PACK_VERSION;       ///< Версия (Сейчас 0x10000 = "0.1.0.0")
    quint32 packType   = PackingType::ptIndividual;    ///< способ паковки: 1 - Индивидуально, 2 - Массив (реализовано только для варианта - 1)
    quint32 dataType   = 0;                            ///< id типа данных  (елемент множества DEPDataType), реализовано только для типов: dpdtFloatValid, dpdtSDWordValid
    quint32 commonTime = DEPTimePoint::dptpUTmsecUTC;   ///< Тип общей метки времени: 0 - нет, или значение из множества DEPTimePoint
    quint32 paramTime  = DEPTimePoint::dptpUTmsecUTC;   ///< Тип индивидуальной метки времени параметров: аналогично предыдущему
    quint32 startIndex = 0;                            ///< Начальный индекс для паковки последовательным массивом (иначе 0)
    quint32 paramCount = 0;                            ///< Число параметров

    QString toString() const;
    void fromDataStream(QDataStream& stream);
    void toStream(QDataStream& stream);

    /// инициализировать заголовок перед отправкой пакета
    void prepare(int p_type, int p_count, quint32 pos = 0);
};

/// DEP data parameter section
struct DEPDataRecord
{
    enum { REC_SIZE = 20 };

    quint32   pack_index = 0;  ///< служебный индекс, указывается в конфиге, для совместимости с принимающей стороной
    QDateTime dt;              ///< временная метка
    QVariant  value;           ///< непосредственно само значение параметра
    qint32    validity   = 0;  ///< достоверность значения параметра

    QString toString() const {
        const QString s_time = dt.toString("dd.MM.yyyy hh:mm:ss.zzz");
        return QString("DEPDataRecord: {%1, %2, %3, %4}")
                .arg(pack_index).arg(s_time, value.toString()).arg(validity);
    }

    void fromDataStream(QDataStream& stream, const DEPDataHeader &i_header);
};

typedef QList<DEPDataRecord> DEPDataRecords;

/**
 * @brief Структура внутреннего пакета с данными
 * Тип данных параметров лежащих в пакете указывается в поле DEPDataHeader.data_type
 */
struct DEPData {
    DEPDataHeader  header;     ///< внутренний заголовок DEPDataHeader
    QDateTime      commonTime; ///< локальное время компьюьера (переменная w32_time_us)
    DEPDataRecords records;    ///< N-секций параметров по 20 байт
};

/**
 * @brief структура полного пакета DEP протокола
 * В одном пакете могут быть только данные одного типа (из множества DEPDataType).
 * Если передаваемых типов параметров несколько, то на каждый тип необходимо создавать свой пакет.
 */
struct DEP {
    DEPHeader header; ///< внешний заголовок DEPHeader (32 bytes)
    DEPData   data;   ///< DEP данные (внутренний пакет)
    quint32   cs;     ///< КС всего пакета (4 bytes)

    int size() const {
        return header.packetSize();
    }
};

#endif //DEP_STRUCTS_H
