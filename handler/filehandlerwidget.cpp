#include "filehandlerwidget.h"
#include "ui_filehandlerwidget.h"

#include <QFileDialog>

/********************************************************/

FileHandlerWidget::FileHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::FileHandlerWidget)
{
    ui->setupUi(this);
    setSettings(SettingsMap());
}

/********************************************************/

FileHandlerWidget::~FileHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap FileHandlerWidget::settings() const
{
    SettingsMap map;
    map.insert(Settings::FileName, ui->editFile->text());
    map.insert(Settings::AppendToFile, ui->chkAppend->isChecked());
    return map;
}

/********************************************************/

void FileHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editFile->setText(map.value(Settings::FileName).toString());
    ui->chkAppend->setChecked(map.value(Settings::AppendToFile, true).toBool());
}

/********************************************************/

void FileHandlerWidget::on_btnFile_clicked()
{
    QString dirPath = QCoreApplication::applicationDirPath();
    QString fileName = ui->editFile->text();
    const QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        dirPath = fileInfo.path();
    }

    fileName = QFileDialog::getSaveFileName(this, tr("Choose File"), dirPath);
    if (!fileName.isEmpty()) {
        ui->editFile->setText(fileName);
    }
}

/********************************************************/
