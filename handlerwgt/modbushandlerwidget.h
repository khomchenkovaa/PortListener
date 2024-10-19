#ifndef MODBUSHANDLERWIDGET_H
#define MODBUSHANDLERWIDGET_H

#include "modbushandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class ModbusHandlerWidget;
}

class ModbusHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit ModbusHandlerWidget(QWidget *parent = nullptr);
    ~ModbusHandlerWidget();

    SettingsMap settings() const;
    void setSettings(const SettingsMap& map);

private slots:
    void openAoCsvFileDialog();
    void openDoCsvFileDialog();
    void openOutputFileDialog();

private:
    Ui::ModbusHandlerWidget *ui;
};

#endif // MODBUSHANDLERWIDGET_H
