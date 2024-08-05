#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include "message.h"

#include <QObject>
#include <QVariant>

typedef QMap<int, QVariant> SettingsMap;
class MessageHandlerWgt;

class MessageHandler : public QObject {
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

    virtual void handleMessage(Message *msg) {
        if (msg->payload.type() == QVariant::ByteArray) {
            processData(msg->payload.toByteArray());
        } else {
            processData(msg->payload.toString());
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

private:
    MessageHandlerPrivate d;
};

#endif // MESSAGEHANDLER_H
