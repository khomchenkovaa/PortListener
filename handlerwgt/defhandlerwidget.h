#ifndef DEFHANDLERWIDGET_H
#define DEFHANDLERWIDGET_H

#include "defhandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class DefHandlerWidget;
}

class DefHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit DefHandlerWidget(QWidget *parent = nullptr);
    ~DefHandlerWidget();

    SettingsMap settings() const;
    void setSettings(const SettingsMap& map);

private slots:
    void openDefFileDialog();

private:
    Ui::DefHandlerWidget *ui;
};

#endif // DEFHANDLERWIDGET_H
