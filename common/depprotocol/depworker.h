#ifndef DEP_WORKER_H
#define DEP_WORKER_H

#include "depenums.h"
#include "depstructs.h"

#include <QObject>
#include <QDataStream>

struct DEPHeader;
struct DEPInternalHeader;

//DEPWorker
class DEPWorker : public QObject
{
    Q_OBJECT

    struct DEPWorkerData {
        QByteArray buffer;           ///< receiving bytes
        QDataStream::FloatingPointPrecision precision = QDataStream::SinglePrecision;
        QDataStream::ByteOrder byteOrder = QDataStream::LittleEndian;
        quint32    packCS;           ///< контрольная сумма последнего пакета целиком
    };

public:
    DEPWorker(QObject *parent = NULL) : QObject(parent) {
        setObjectName("depworker_obj");
    }

    QString name() const {
        return QString("DEP worker object");
    }

    /// добавить новые пришедшие байты в  текущий буфер m_buffer, затем выполнить work()
    void addToBuffer(const QByteArray& ba) {
        d.buffer.append(ba);
        work();
    }

    /// устанавливает порядок байт для чтения из потока
    inline void setByteOrder(int b_order) {
        d.byteOrder = static_cast<QDataStream::ByteOrder>(b_order);
    }
    inline int curByteOrder() const {
        return d.byteOrder;
    }

    /// создать готовый пакет для отправки со значениями типа dpdtFloatValid
    QByteArray makeFloatValidPacket(const QList<quint16>& indexes, const QByteArray& view_ba) const {
        return makeDEPPacket(indexes, view_ba, dpdtFloatValid);
    }

    /// создать готовый пакет для отправки со значениями типа dpdtSDWordValid
    QByteArray makeIntValidPacket(const QList<quint16>& indexes, const QByteArray& view_ba) const {
        return makeDEPPacket(indexes, view_ba, dpdtSDWordValid);
    }

signals:
    /**
     * @brief эмитится когда был упешно спарсен полностью очередной пришедшедший пакет
     * далее он конвертируется в спец. QByteArray - формат понятный для вьюхи (xmlpack lib)
     */
    void signalRewriteReceivedPacket(const QList<quint16>&, const QByteArray&);
    void dataReceived(const QList<DEPDataRecord>& data);
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
    void work(bool try_next = false);

    // функции, которые выполняются при считывании пакетов из m_buffer
    /// проверка контрольной суммы текущего читаемого пакета целиком
    bool isPackChecksumOk(const DEPHeader& header);

    /// попытаться найти валидный заголовок в начале m_buffer, 2-й парметр результат поиска
    DEPHeader tryGetHeader(bool& ok);

    /// извлечь данные из тела пакета (т.е. без обертки)
    void parseBodyPacket(const QByteArray& ba);

    /// извлечь данные после заголовка из тела пакета (без обертки)
    void parseDataPacket(const QByteArray& ba, const DEPInternalHeader&i_header);

    /// распарсить данные типа dpdtFloatValid
    void readData(const QByteArray& ba, const DEPInternalHeader& i_header);

    /// распарсить данные типа dpdtSDWordValid
    void readSDWordValidData(const QByteArray& ba, const DEPInternalHeader& i_header);

    // make sending packets funcs
    /// обернуть готовый внутренний пакет в DEPHeader и checksum
    void wrapPacket(QByteArray&) const;

    /// make DEP packet by data type
    QByteArray makeDEPPacket(const QList<quint16>&, const QByteArray&, int) const;

private:
    DEPWorkerData d;
};

#endif //DEP_WORKER_H
