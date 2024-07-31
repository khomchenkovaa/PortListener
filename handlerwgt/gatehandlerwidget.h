#ifndef GATEHANDLERWIDGET_H
#define GATEHANDLERWIDGET_H

#include "gatehandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class GateHandlerWidget;
}

class GateHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit GateHandlerWidget(QWidget *parent = nullptr);
    ~GateHandlerWidget();

    SettingsMap settings() const;
    void setSettings(const SettingsMap& map);

private slots:
    void openCsvFileDialog();
    void openDefFileDialog();
    void openOutputFileDialog();

private:
    Ui::GateHandlerWidget *ui;
};

#endif // GATEHANDLERWIDGET_H
