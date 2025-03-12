#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <math.h>

#include <QtMath>
#include <QList>
#include <QVector>
#include <QRandomGenerator>
#include <QTime>

/// Разные функции
namespace Utils
{

inline QString alignStrBA(const QString& s, int n, bool with_x)
{
    QString result = s.toUpper();
    if (n > s.length()) {
        QString cs(n - s.length(), QChar('0'));
        result.prepend(cs);
    }
    if (with_x) result.prepend("0x");
    return result;
}

/// развернуть порядок байт в заданном QByteArray
inline void reverseBA(QByteArray& ba) {
    if (ba.size() < 2) return;
    std::reverse(ba.begin(), ba.end());
}

template<typename T>
T valueFromBA(const QByteArray& ba, quint16 pos) {
    T result = 0;
    int sf = sizeof(T);
    if ((pos+sf) <= ba.size()) {
        QByteArray ba_mid(ba.mid(pos, sf));
        reverseBA(ba_mid);
        memcpy(&result, ba_mid.data(), sf);
    }
    return result;
}

/// случайное значение от a до b, при условии a <= b
inline int rndInt(uint a, uint b)
{
    if (a == b) return a;
    if (a > b) return -1;
    return QRandomGenerator::global()->bounded(a, b);
}

/// случайное значение от 0 до 1(всегда меньше 1) с разрешением 4 знака
inline double rnd() {
    return double(rndInt(0, 9999))/10000;
}

/// случайное вещественное значение от a до b, при условии a <= b
inline float rndFloat(float a, float b) {
    if (a == b) return a;
    if (a > b) return -1;
    return (a + rnd()*(b-a));
}

/// установка генератора случайных чисел в случайное положение
inline void rndReset()
{
    QRandomGenerator::global()->seed(QTime::currentTime().msec());
}

/// случайное значение да/нет при заданной вероятности в %
inline bool probabilityOk(float p)
{
    // RAND_MAX == 2 147 483 647
    if (p > 0) {
        p /= float(100);
        float rnd_limit = p*float(RAND_MAX);
        return (QRandomGenerator::global()->generate() < rnd_limit);
    }
    return false;
}

/// возвращает строку в шеснадцатиричном формате значения a, если with_x == true то вернет в формате 0xXXXXXXXX
inline QString uint8ToBAStr(quint8 a, bool with_x = false) {
    return alignStrBA(QString::number(a, 16), sizeof(a)*2, with_x);
}

/// возвращает строку в шеснадцатиричном формате значения a, если with_x == true то вернет в формате 0xXXXXXXXX
inline QString uint16ToBAStr(quint16 a, bool with_x = false) {
    return alignStrBA(QString::number(a, 16), sizeof(a)*2, with_x);
}

/// возвращает строку в шеснадцатиричном формате значения a, если with_x == true то вернет в формате 0xXXXXXXXX
inline QString uint32ToBAStr(quint32 a, bool with_x = false) {
    return alignStrBA(QString::number(a, 16), sizeof(a)*2, with_x);
}

/**
 * взять n байт в заданном QByteArray, начиная с указанной позиции и преобразовать в число
 * если позиция указана неверно или байт не хватает, то вернет 0.
 */
inline quint16 uint16FromBA(const QByteArray& ba, quint16 pos) {
    return valueFromBA<quint16>(ba, pos);
}

} // namespace Utils

#endif // MATHUTILS_H
