#include "iodecoder.h"

#include <QTextCodec>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

/********************************************************/

IODecoder::IODecoder()
{
    mib = 1000; // Unicode
}

/********************************************************/

QString IODecoder::toUnicode(const QByteArray& data, bool binary) const
{
    QString result;
    if (binary) {
        result = QString::fromLatin1(data.toHex());
    } else {
        const QTextCodec *codec = QTextCodec::codecForMib(mib);
        result = codec->toUnicode(data.constData(), data.size());
    }
    return result;
}

/********************************************************/

QByteArray IODecoder::fromUnicode(const QString &data, bool binary) const
{
    QByteArray result;
    if (binary) {
        result = QByteArray::fromHex(data.toLatin1());
    } else {
        const QTextCodec *codec = QTextCodec::codecForMib(mib);
        result = codec->fromUnicode(data);
    }
    return result;
}

/********************************************************/

QList<QTextCodec *> IODecoder::findCodecs()
{
    QMap<QString, QTextCodec *> codecMap;
    QRegularExpression iso8859RegExp("^ISO[- ]8859-([0-9]+).*$");
    QRegularExpressionMatch match;

    foreach (int mib, QTextCodec::availableMibs()) {
        QTextCodec *codec = QTextCodec::codecForMib(mib);

        QString sortKey = codec->name().toUpper();
        int rank;

        if (sortKey.startsWith(QLatin1String("UTF-8"))) {
            rank = 1;
        } else if (sortKey.startsWith(QLatin1String("UTF-16"))) {
            rank = 2;
        } else if ((match = iso8859RegExp.match(sortKey)).hasMatch()) {
            if (match.captured(1).size() == 1)
                rank = 3;
            else
                rank = 4;
        } else {
            rank = 5;
        }
        sortKey.prepend(QLatin1Char('0' + rank));

        codecMap.insert(sortKey, codec);
    }
    return codecMap.values();
}

/********************************************************/
