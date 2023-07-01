#ifndef TCPHANDLERWIDGET_H
#define TCPHANDLERWIDGET_H

#include "tcphandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class TcpHandlerWidget;
}

class TcpHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit TcpHandlerWidget(QWidget *parent = nullptr);
    ~TcpHandlerWidget();

    SettingsMap settings() const;
    void setSettings(const SettingsMap& map);

private:
    Ui::TcpHandlerWidget *ui;
};

#endif // TCPHANDLERWIDGET_H
