#ifndef SOCKHANDLERWIDGET_H
#define SOCKHANDLERWIDGET_H

#include "sockhandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class SockHandlerWidget;
}

class SockHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit SockHandlerWidget(QWidget *parent = nullptr);
    ~SockHandlerWidget();

    SettingsMap settings() const;
    void setSettings(const SettingsMap& map);

private:
    Ui::SockHandlerWidget *ui;
};

#endif // SOCKHANDLERWIDGET_H
