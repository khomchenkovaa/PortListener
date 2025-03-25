#ifndef DEPHANDLERWIDGET_H
#define DEPHANDLERWIDGET_H

#include "dephandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class DepHandlerWidget;
}

class DepHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit DepHandlerWidget(QWidget *parent = nullptr);
    ~DepHandlerWidget();

    SettingsMap settings() const;
    void setSettings(const SettingsMap& map);

private slots:
    void openCsvFileDialog();
    void onTypeValueCmbChanged();
    void openOutputFileDialog();

private:
    Ui::DepHandlerWidget *ui;
};

#endif // DEPHANDLERWIDGET_H
