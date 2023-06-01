#ifndef MESSAGEHANDLERWGT_H
#define MESSAGEHANDLERWGT_H

#include "messagehandler.h"

#include <QWidget>

class MessageHandlerWgt : public QWidget
{
    Q_OBJECT

public:
    explicit MessageHandlerWgt(QWidget *parent = nullptr) : QWidget(parent) {
        setObjectName("HandlerWidget");
    }

    virtual SettingsMap settings() const {
        return SettingsMap();
    }

    virtual void setSettings(const SettingsMap& map) {
        Q_UNUSED(map)
    }
};

#endif // MESSAGEHANDLERWGT_H
