#ifndef IODECODER_H
#define IODECODER_H

#include <QString>

class IODecoder
{
public:
    explicit IODecoder();

    QString toUnicode(const QByteArray& data, bool binary = false) const;
    QByteArray fromUnicode(const QString& data, bool binary = false) const;

    void setMib(int mib) {
        this->mib = mib;
    }

public:
    static QList<QTextCodec *> findCodecs();

private:
   int mib;
};

#endif // IODECODER_H
