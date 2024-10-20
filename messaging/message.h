#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QVariant>
#include <QUuid>
#include <QDateTime>
#include <QSharedPointer>

/**
 * A message representation with headers and body
 */
struct Message {
    QUuid           id        = QUuid::createUuid();
    QDateTime       timestamp = QDateTime::currentDateTime();
    QString         sender;
    QVariant        payload;
    QMetaType::Type payloadType;
    QVariant        original;
    QVariantMap     headers;
};

typedef QSharedPointer<Message> PMessage;

#endif // MESSAGE_H
