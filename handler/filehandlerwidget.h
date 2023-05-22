#ifndef FILEHANDLERWIDGET_H
#define FILEHANDLERWIDGET_H

#include "ihandler.h"

namespace Ui {
class FileHandlerWidget;
}

class FileHandlerWidget : public IHandlerWidget
{
    Q_OBJECT

public:
    enum Settings {
        FileName,
        AppendToFile
    };

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
