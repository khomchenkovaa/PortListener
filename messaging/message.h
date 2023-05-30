#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QVariant>
#include <QUuid>
#include <QDateTime>

/**
 * A message representation with headers and body
 */
struct Message {

    QUuid       id;
    QDateTime   timestamp;
    QString     sender;
    QVariant    payload;
    QVariant    original;
    QVariantMap headers;

    explicit Message(const QString& sndr, const QVariant& value, const QVariantMap& hdrs = QVariantMap())
        : id(QUuid::createUuid())
        , timestamp(QDateTime::currentDateTime())
        , sender(sndr)
        , payload(value)
        , original(value)
        , headers(hdrs)
    {
    }
};

#endif // MESSAGE_H
