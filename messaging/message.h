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
    QVariant        payload;
    QMetaType::Type payloadType;
    QVariantMap     headers;
};

typedef QSharedPointer<Message> PMessage;

#endif // MESSAGE_H
