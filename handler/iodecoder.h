#ifndef IODECODER_H
#define IODECODER_H

#include <QString>

#define MIB_UNICODE 1000

class IODecoder
{
public:
    explicit IODecoder(int mib = MIB_UNICODE);

    QString toUnicode(const QByteArray& data, bool binary = false) const;
    QByteArray fromUnicode(const QString& data, bool binary = false) const;

public:
    static QList<QTextCodec *> findCodecs();

private:
   int mib;
};

#endif // IODECODER_H
