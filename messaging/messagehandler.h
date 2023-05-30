#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include "message.h"

#include <QObject>
#include <QVariant>

typedef QMap<int, QVariant> SettingsMap;
class MessageHandlerWgt;

class MessageHandler : public QObject {
    Q_OBJECT

public:
    explicit MessageHandler(QObject *parent = nullptr) : QObject(parent) {
        m_Name = tr("Empty handler");
        m_Connected = false;
    }

    QString name() const {
        return m_Name;
    }

    virtual void processMessage(Message *msg) {
        Q_UNUSED(msg)
    }

    virtual QByteArray processData(const QByteArray& data) {
        Q_UNUSED(data)
        return QByteArray();
    }

    virtual QByteArray processData(const QString& data) {
        Q_UNUSED(data)
        return QByteArray();
    }

    SettingsMap settings() const {
        return m_Settings;
    }

    void setSettings(const SettingsMap& map) {
        m_Settings.clear();
        QMapIterator<int, QVariant> i(map);
        while (i.hasNext()) {
            i.next();
            m_Settings.insert(i.key(), i.value());
        }
    }

    virtual void connect(bool binary = false) {
        Q_UNUSED(binary)
        m_Connected = true;
    }

    virtual void disconnect() {
        m_Connected = false;
    }

    virtual MessageHandlerWgt *settingsWidget(QWidget *parent = nullptr) const {
        Q_UNUSED(parent)
        return Q_NULLPTR;
    }

    bool hasError() const {
        return !m_Error.isEmpty();
    }

    QString lastError() const {
        return m_Error;
    }

protected:
    QString m_Name;
    SettingsMap m_Settings;
    bool m_Connected;
    QString m_Error;
};

#endif // MESSAGEHANDLER_H
