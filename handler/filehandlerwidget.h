#ifndef FILEHANDLERWIDGET_H
#define FILEHANDLERWIDGET_H

#include "filehandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class FileHandlerWidget;
}

class FileHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit FileHandlerWidget(QWidget *parent = nullptr);
    ~FileHandlerWidget();

    SettingsMap settings() const;

    void setSettings(const SettingsMap& map);

private slots:
    void on_btnFile_clicked();

private:
    Ui::FileHandlerWidget *ui;
};

#endif // FILEHANDLERWIDGET_H
