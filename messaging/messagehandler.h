#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include "message.h"

#include <QObject>
#include <QVariant>
#include <QMetaMethod>

typedef QMap<int, QVariant> SettingsMap;
class MessageHandlerWgt;

class MessageHandler : public QObject
{
    Q_OBJECT

    struct MessageHandlerPrivate {
        QString     name;              ///< Handler's name
        SettingsMap settings;          ///< Handler's settings
        bool        connected = false; ///< Connected flag
        QStringList errors;            ///< Error strings
    };

public:
    explicit MessageHandler(const QString &name = tr("Empty handler"), QObject *parent = nullptr) : QObject(parent) {
        d.name = name;
    }

    virtual QByteArray handleMessage(PMessage msg) {
        if (msg->payload.type() == QVariant::ByteArray) {
            return processData(msg->payload.toByteArray());
        } else {
            return processData(msg->payload.toString());
        }
    }

    virtual QByteArray processData(const QByteArray& data) {
        Q_UNUSED(data)
        clearErrors();
        return QByteArray();
    }

    virtual QByteArray processData(const QString& data) {
        Q_UNUSED(data)
        clearErrors();
        return QByteArray();
    }

    virtual void doConnect(bool binary = false) {
        Q_UNUSED(binary)
        setConnected();
    }

    virtual void doDisconnect() {
        setDisconnected();
    }

    virtual bool allowBinary() const {
        return true;
    }

    virtual bool allowText() const {
        return false;
    }

    QString name() const {
        return d.name;
    }

    SettingsMap *settings() {
        return &(d.settings);
    }

    void setSettings(const SettingsMap& map) {
        d.settings.clear();
        for (auto i = map.constBegin(); i != map.constEnd(); i++) {
            d.settings.insert(i.key(), i.value());
        }
    }

    bool isConnected() const {
        return d.connected;
    }

    void setConnected() {
        d.connected = true;
    }

    void setDisconnected() {
        d.connected = false;
        disconnect(this, QMetaMethod(), Q_NULLPTR, QMetaMethod());
    }

    bool hasError() const {
        return !d.errors.isEmpty();
    }

    QString lastError() const {
        if (hasError()) {
            return d.errors.constLast();
        }
        return QString();
    }

    QStringList errors() const {
        return d.errors;
    }

    void addError(const QString &error) {
        d.errors << error;
    }

    void clearErrors() {
        d.errors.clear();
    }

Q_SIGNALS:
    void logMessage(const QString &msgHandler, const QString &msg);
    void logError(const QString &msgHandler, const QString &msg);

private:
    MessageHandlerPrivate d;
};

#endif // MESSAGEHANDLER_H
