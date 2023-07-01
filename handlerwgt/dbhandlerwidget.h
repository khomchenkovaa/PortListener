#ifndef DBHANDLERWIDGET_H
#define DBHANDLERWIDGET_H

#include "dbhandler.h"
#include "messagehandlerwgt.h"

namespace Ui {
class DbHandlerWidget;
}

class DbHandlerWidget : public MessageHandlerWgt
{
    Q_OBJECT

public:
    explicit DbHandlerWidget(QWidget *parent = nullptr);
    ~DbHandlerWidget();

    SettingsMap settings() const;
    void setSettings(const SettingsMap& map);

private slots:
    void on_comboType_currentIndexChanged(int index);
    void on_buttonSelectFile_clicked();
    void on_btnTest_clicked();

private:
    void updateFields(const QString& drv);

private:
    static QString getDescription(const QString& drv);

private:
    Ui::DbHandlerWidget *ui;
};

#endif // DBHANDLERWIDGET_H
