#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#include <QDateTime>
#include <QStringList>
#include <QDataStream>

enum class MSECS : quint64 {
    BEFORE_UNIX = 11644473600000ULL
};

/**
 * @brief The w32_time struct
 * Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
 * 11644473600 second before unix epoch (so = (unixtime + 11644473600) * 10000
 */
struct w32_time
{
    quint32 dwLow  = 100;
    quint32 dwHigh = 100;

    /// вернет свой размер в байтах
    inline quint32 size() const {
        return (sizeof(dwLow) + sizeof(dwHigh));
    }

    /// записать структуру в поток
    void toStream(QDataStream& stream) {
        stream << dwHigh << dwLow;
    }

    /// считать структуру из потока
    void fromStream(QDataStream& stream) {
        stream >> dwHigh >> dwLow;
    }

    /// конвертирует QDateTime в w32_time
    void setTime(const QDateTime& dt) {
        quint64 tmp = quint64(dt.toTime_t())*quint64(1000); //конвертация dt в количество мсек, прошедших с (1970-01-01 00:00:00)
        tmp += static_cast<quint64>(MSECS::BEFORE_UNIX); // before Unix msecs
        tmp *= 10000ULL; // конвертация милисекунд в 100-наносекундные интервалы;

        dwHigh = (tmp >> 32); //извлечение старших 4-х байтов из значения tmp
        dwLow = ((tmp << 32) >> 32); //извлечение младших 4-х байтов из значения tmp
    }

    /// вернет значения полей структуры в виде строки
    QString toStr() const {
        QString s_time = toQDateTime().toString("dd.MM.yyyy  hh:mm:ss.zzz");
        return QString("W32_TIME: dwLow=%1 dwHigh=%2 (%3)").arg(dwLow).arg(dwHigh).arg(s_time);
    }

    /// преобразование структуры в значение QDateTime
    QDateTime toQDateTime(Qt::TimeSpec ts = Qt::UTC) const {
        quint64 tmp = (quint64(dwHigh)*0x100000000ULL + quint64(dwLow))/10000ULL; // конвертация структуры в одно значение quint64, (мсек)
        if (tmp < static_cast<quint64>(MSECS::BEFORE_UNIX)) {
            // предупреждение: это время меньше (1970-01-01 00:00:00)
            return QDateTime();
        } else {
            // вычитаем количество мсек начиная с начала эпохи unix
            tmp -= static_cast<quint64>(MSECS::BEFORE_UNIX);
        }

        QDateTime result;
        result.setTimeSpec(ts);
        result.setTime_t(uint(tmp/1000ULL)); //Устанавливает дату и время, параметр - количество секунд, прошедших с (1970-01-01 00:00:00)
        return result.addMSecs(tmp%1000ULL); //добавляет остаток мсек не кратных одной секунде
    }

    /// устанавливает текущие дату и время
    void setCurrentTime() {
        setTime(QDateTime::currentDateTime());
    }

    /// устанавливает текущие дату и время (UTC)
    void setCurrentTimeUtc() {
        setTime(QDateTime::currentDateTimeUtc());
    }
};

/**
 * @brief The w32_time_us struct
 * содержит 2 значения:
 * в старшем количество секунд с начала эпохи unix (1970-01-01 00:00:00)
 * в младшем количество микросекунд последней(текущей) секунды
 */
struct w32_time_us
{
    quint32 dwLow  = 100;
    quint32 dwHigh = 100;

    /// вернет свой размер в байтах
    inline quint32 size() const {
        return (sizeof(dwLow) + sizeof(dwHigh));
    }

    /// записать структуру в поток
    void toStream(QDataStream& stream) {
        stream << dwHigh << dwLow;
    }

    /// считать структуру из потока
    void fromStream(QDataStream& stream) {
        stream >> dwHigh >> dwLow;
    }

    /// конвертирует QDateTime в w32_time_us
    void setTime(const QDateTime& dt) {
        quint64 msecs = dt.toMSecsSinceEpoch();
        quint32 msecs_32 = quint32(msecs%1000);
        dwLow = msecs_32*1000;
        dwHigh = quint32((msecs - msecs_32)/1000);
    }

    /// вернет значения полей структуры в виде строки
    QString toStr() const {
        QString s_time = toQDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
        return QString("W32_TIME_US: dwLow=%1 dwHigh=%2 (%3)").arg(dwLow).arg(dwHigh).arg(s_time);
    }

    /// преобразование структуры в значение QDateTime
    QDateTime toQDateTime(Qt::TimeSpec ts = Qt::UTC) const {
        quint64 tmp = (quint64(dwHigh)*1000 + quint64(dwLow)/1000); // конвертация структуры в одно значение quint64, (мсек)
        QDateTime dt;
        dt.setMSecsSinceEpoch(tmp);
        dt.setTimeSpec(ts);
        return dt;
    }

    /// устанавливает текущие дату и время
    void setCurrentTime() {
        setTime(QDateTime::currentDateTime());
    }

    /// устанавливает текущие дату и время (UTC)
    void setCurrentTimeUtc() {
        setTime(QDateTime::currentDateTimeUtc());
    }
};

/// system datetime c++
struct w32_system_time
{
    qint16 wYear         = 2020;
    qint16 wMonth        = 9;
    qint16 wDayOfWeek    = 2;
    qint16 wDay          = 1;
    qint16 wHour         = 12;
    qint16 wMinute       = 0;
    qint16 wSecond       = 0;
    qint16 wMilliseconds = 0;

    /// записать структуру в поток
    void toStream(QDataStream &stream) {
        stream << wYear << wMonth << wDayOfWeek << wDay
               << wHour << wMinute << wSecond << wMilliseconds;
    }

    /// конвертирует QDateTime в w32_system_time
    void setTime(const QDateTime& dt) {
        wYear         = dt.date().year();
        wMonth        = dt.date().month();
        wDayOfWeek    = dt.date().dayOfWeek();
        wDay          = dt.date().day();
        wHour         = dt.time().hour();
        wMinute       = dt.time().minute();
        wSecond       = dt.time().second();
        wMilliseconds = dt.time().msec();
    }

    /// задает текущие дату и время в w32_system_time
    void setCurrentTime() {
        setTime(QDateTime::currentDateTime());
    }

    /// вернет свой размер в байтах
    quint32 size() const {
        return (sizeof(wYear) + sizeof(wMonth) + sizeof(wDayOfWeek) + sizeof(wDay) +
                sizeof(wHour) + sizeof(wMinute) + sizeof(wSecond) + sizeof(wMilliseconds));
    }

    QDate date() const {
        return QDate(wYear, wMonth, wDay);
    }

    QTime time() const {
        return QTime(wHour, wMinute, wSecond, wMilliseconds);
    }

    /// преобразование структуры в значение QDateTime
    QDateTime toQDateTime(Qt::TimeSpec ts = Qt::UTC) const {
        return QDateTime(date(), time(), ts);
    }
};

struct timespec;

/// static funcs time
namespace Utils
{
/// вернет текущее время в виде строки в заданном формате
inline QString strCurrentTime(QString mask = "hh:mm:ss_zzz") {
    return QTime::currentTime().toString(mask);
}

/// вернет текущее время и дату в виде строки в заданном формате
inline QString strCurrentDateTime(QString mask = "dd.MM.yyyy hh:mm:ss") {
    return QDateTime::currentDateTime().toString(mask);
}

/// врернет смещение локального времени относительно UTC (в часах) (Qt5)
inline int utcOffset() {
    QDateTime t_loc(QDateTime::currentDateTime());
    t_loc.setTimeSpec(Qt::LocalTime);
    QDateTime t_utc(QDateTime::currentDateTime());
    t_loc.setTimeSpec(Qt::UTC);
    return (t_utc.secsTo(t_loc)/3600);
}

/**
 * @brief функция запишет в переменную tm(сишная структура) текущее значение времени
 * @param tm результат
 * @param ts указывает какой часовой пояс нужен (utc либо местное время)
 * @param def_nsec если >= 0, то в поле tv_nsec запишется принудительно это значение
 */
inline void getTimeSpecCPP(timespec &tm, Qt::TimeSpec ts = Qt::UTC, qint64 def_nsec = -1) {
#ifdef OS_CENTOS
    clock_gettime (CLOCK_REALTIME, &tm);
#else
    timespec_get(&tm, TIME_UTC);
#endif
    if (ts == Qt::LocalTime) tm.tv_sec += (3600 * utcOffset());
    if (def_nsec >= 0) tm.tv_nsec = def_nsec;
}

/// вернет параметр timespec в виде строки в заданном формате
inline QString strTimeSpec(const timespec &tm, Qt::TimeSpec ts = Qt::UTC, QString mask = "dd.MM.yyyy hh:mm:ss.zzz") {
    qint64 tmp = (tm.tv_sec*1000) + (tm.tv_nsec/1000000);
    QDateTime dt;
    dt.setMSecsSinceEpoch(tmp);
    dt.setTimeSpec(ts);
    return dt.toString(mask);
}

} // namespace Utils

#endif //TIMEUTILS_H
