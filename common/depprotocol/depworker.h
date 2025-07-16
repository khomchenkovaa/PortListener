#ifndef DEP_WORKER_H
#define DEP_WORKER_H

#include "depenums.h"
#include "depstructs.h"

#include <QObject>
#include <QDataStream>

struct DEPHeader;
struct DEPDataHeader;

class DEPWorker : public QObject
{
    Q_OBJECT

    struct DEPWorkerData {
        QByteArray buffer;    ///< receiving bytes
        QDataStream::FloatingPointPrecision precision = QDataStream::SinglePrecision;
        QDataStream::ByteOrder byteOrder = QDataStream::LittleEndian;
        DEP packet;           ///< received packet
    };

public:
    DEPWorker(QObject *parent = NULL) : QObject(parent) {
        setObjectName("depworker_obj");
    }

    QString name() const {
        return "DEP worker object";
    }

    DEPData packetData() const {
        return d.packet.data;
    }

    /// добавить новые пришедшие байты в текущий буфер
    void addToBuffer(const QByteArray& ba) {
        d.buffer.append(ba);        
    }

    bool isBufferEmpty() const {
        return d.buffer.isEmpty();
    }

    bool doWork() {
        return work();
    }

    /// устанавливает порядок байт для чтения из потока
    void setByteOrder(int b_order) {
        d.byteOrder = static_cast<QDataStream::ByteOrder>(b_order);
    }
    int curByteOrder() const {
        return d.byteOrder;
    }

    /// \return тип данных DEPDataType последнего успешно спарсенного пакета
    int lastReceivedType() const {
        return d.packet.data.header.dataType;
    }

    /// создать готовый пакет для отправки со значениями типа dpdtFloatValid
    QByteArray makeFloatValidPacket(const QList<quint16> &indexes, const QByteArray &view_ba) const {
        return makeDEPPacket(indexes, view_ba, dpdtFloatValid);
    }

    /// создать готовый пакет для отправки со значениями типа dpdtFloatValid
    QByteArray makeFloatValidPacket(const DEPDataRecords &data) const {
        return makeDEPPacket(data, dpdtFloatValid);
    }

    /// создать готовый пакет для отправки со значениями типа dpdtSDWordValid
    QByteArray makeIntValidPacket(const QList<quint16> &indexes, const QByteArray &view_ba) const {
        return makeDEPPacket(indexes, view_ba, dpdtSDWordValid);
    }

    /// создать готовый пакет для отправки со значениями типа dpdtSDWordValid
    QByteArray makeIntValidPacket(const DEPDataRecords &data) const {
        return makeDEPPacket(data, dpdtSDWordValid);
    }

signals:
    void dataReceived(const DEPData& data);
    void signalError(const QString& msg);
    void signalMsg(const QString& msg);

private:
    /**
     * @brief главная функция для разбора приходящих DEP-пакетов.
     * обработать текущий полученный массив байт следующим образом:
     * если хватает данных для внешнего заголовка(DEPHeader), то считывается заголовок,
     * затем если хватает данных(размер в DEPHeader - len) на весь пакет, то считывается весь пакет и целиком удаляется из m_buffer
     * если на весь пакет байт не хватает то  ждем следующего цикла и заново считываем тот же заголовок и т.д.
     */
    bool work(bool try_next = false);

    /// проверка контрольной суммы текущего читаемого пакета целиком
    bool isPackChecksumOk();

    /// попытаться найти валидный заголовок в начале буфера
    DEPHeader tryGetHeader(bool &ok);

    /// извлечь данные из тела пакета (т.е. без обертки)
    DEPData parseBodyPacket(const QByteArray& ba);

    /// извлечь данные после заголовка из тела пакета
    DEPDataRecords readData(const QByteArray &ba, const DEPDataHeader &i_header);

    /// обернуть готовый внутренний пакет в DEPHeader и checksum
    void wrapPacket(QByteArray& packet_ba) const;

    /// make DEP packet by data type
    QByteArray makeDEPPacket(const QList<quint16> &indexes, const QByteArray &view_ba, int p_type) const;

    /// make DEP packet by data type
    QByteArray makeDEPPacket(const DEPDataRecords &data, int p_type) const;

private:
    DEPWorkerData d;
};

#endif //DEP_WORKER_H
