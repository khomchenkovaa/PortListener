#ifndef UDPHANDLERWIDGET_H
#define UDPHANDLERWIDGET_H

#include "udphandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class UdpHandlerWidget;
}

class UdpHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit UdpHandlerWidget(QWidget *parent = nullptr);
    ~UdpHandlerWidget();

    SettingsMap settings() const;
    void setSettings(const SettingsMap& map);

private:
    Ui::UdpHandlerWidget *ui;
};

#endif // UDPHANDLERWIDGET_H
