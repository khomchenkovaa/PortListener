#ifndef IHANDLER_H
#define IHANDLER_H

#include <QObject>
#include <QVariant>
#include <QWidget>

/********************************************************/

typedef QMap<int, QVariant> SettingsMap;
class IHandlerWidget;

/********************************************************/

class IHandler : public QObject {
    Q_OBJECT

public:
    explicit IHandler(QObject *parent = nullptr) : QObject(parent) {
        m_Name = tr("Empty handler");
        m_Connected = false;
    }

    QString name() const {
        return m_Name;
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

    virtual IHandlerWidget *settingsWidget(QWidget *parent = nullptr) const {
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

/********************************************************/

class IHandlerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IHandlerWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setObjectName("HandlerWidget");
    }

    virtual SettingsMap settings() const {
        return SettingsMap();
    }

    void setSettings(const SettingsMap& map) {
        Q_UNUSED(map)
    }
};

/********************************************************/

#endif // IHANDLER_H
