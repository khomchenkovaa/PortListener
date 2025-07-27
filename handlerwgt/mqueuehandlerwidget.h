#ifndef MQUEUEHANDLERWIDGET_H
#define MQUEUEHANDLERWIDGET_H

#include "mqueuehandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class MqueueHandlerWidget;
}

class MqueueHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit MqueueHandlerWidget(QWidget *parent = nullptr);
    ~MqueueHandlerWidget();

    SettingsMap settings() const;
    void setSettings(const SettingsMap& map);

private:
    Ui::MqueueHandlerWidget *ui;
};

#endif // MQUEUEHANDLERWIDGET_H
