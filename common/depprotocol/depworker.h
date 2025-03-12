#ifndef DEP_WORKER_H
#define DEP_WORKER_H

#include <QObject>

class QDataStream;
struct DEPHeader;
struct DEPInternalHeader;

//DEPWorker
class DEPWorker : public QObject
{
    Q_OBJECT

    struct DEPWorkerData {
        QByteArray buffer;           ///< receiving bytes
        int        byteOrder   = 0;
        quint32    packCS;           ///< контрольная сумма последнего пакета целиком
        int        lastSigType = -1; ///< переменная множества DEPDataType, хранит тип данных последнего успешно спарсенного пришедшего пакета
    };

public:
    DEPWorker(QObject *parent = NULL);

    QString name() const {
        return QString("DEP worker object");
    }

    /// добавить новые пришедшие байты в  текущий буфер m_buffer, затем выполнить work()
    void addToBuffer(const QByteArray&);

    /// устанавливает порядок байт для чтения из потока
    inline void setByteOrder(int b_order) {
        d.byteOrder = b_order;
    }
    inline int buffSize() const {
        return d.buffer.size();
    }
    inline bool bufferEmpty() const {
        return d.buffer.isEmpty();
    }
    inline int lastReceivedType() const {
        return d.lastSigType;
    }
    inline int curByteOrder() const {
        return d.byteOrder;
    }

    /// создать готовый пакет для отправки со значениями типа dpdtFloatValid
    QByteArray makeFloatValidPacket(const QList<quint16>&, const QByteArray&) const;

    /// создать готовый пакет для отправки со значениями типа dpdtSDWordValid
    QByteArray makeIntValidPacket(const QList<quint16>&, const QByteArray&) const;

    static quint16 depChecksumSize() {
        return sizeof(uint32_t);
    }

signals:
    /**
     * @brief эмитится когда был упешно спарсен полностью очередной пришедшедший пакет
     * далее он конвертируется в спец. QByteArray - формат понятный для вьюхи (xmlpack lib)
     */
    void signalRewriteReceivedPacket(const QList<quint16>&, const QByteArray&);
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
    /// проверка контрольной суммы внешнего заголовка, стоящего в начале m_buffer
    bool headerChecksumOk(const DEPHeader&) const;

    /// проверка контрольной суммы текущего читаемого пакета целиком
    bool packChecksumOk(const DEPHeader&) const;

    /// размер текущего читаемого пакета целиком (DEPHeader + body + cs)
    int lastPackSize(const DEPHeader&) const;

    /// попытаться найти валидный заголовок в начале m_buffer, 2-й парметр результат поиска
    void tryGetHeader(DEPHeader&, bool&);

    /// попытаться считать оставшееся тело пакета после заголовка из m_buffer
    void tryGetBody(const DEPHeader&, bool&);

    /// извлечь данные из тела пакета (т.е. без обертки)
    void parseBodyPacket(const QByteArray&);

    /// извлечь данные после заголовка из тела пакета (без обертки)
    void parseDataPacket(const QByteArray&, const DEPInternalHeader&, QDataStream&);

    /// распарсить данные типа dpdtFloatValid
    void readFloatValidData(const QByteArray&, const DEPInternalHeader&, QDataStream&);

    /// распарсить данные типа dpdtSDWordValid
    void readSDWordValidData(const QByteArray&, const DEPInternalHeader&, QDataStream&);

    // make sending packets funcs
    /// обернуть готовый внутренний пакет в DEPHeader и checksum
    void wrapPacket(QByteArray&) const;

    /// make DEP packet by data type
    QByteArray makeDEPPacket(const QList<quint16>&, const QByteArray&, int) const;

    /// в случае ошибки чтения заголовка надо отрезать первое значение сначала m_buffer для повторной попытки
    void cutOutFirstValue();

    /// установить параметры потока данных для чтения
    void prepareStream(QDataStream&, int skip_bytes = 0) const;

private:
    DEPWorkerData d;
};

#endif //DEP_WORKER_H
