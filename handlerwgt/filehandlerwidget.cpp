#include "filehandlerwidget.h"
#include "ui_filehandlerwidget.h"

#include <QFileDialog>

/********************************************************/

FileHandlerWidget::FileHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::FileHandlerWidget)
{
    ui->setupUi(this);
    FileHandlerWidget::setSettings(SettingsMap());
    connect(ui->btnFile, &QAbstractButton::clicked,
            this, &FileHandlerWidget::openFileDialog);
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
    map.insert(FileHandler::FileName, ui->editFile->text());
    map.insert(FileHandler::FileAppend, ui->chkAppend->isChecked());
    return map;
}

/********************************************************/

void FileHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editFile->setText(map.value(FileHandler::FileName).toString());
    ui->chkAppend->setChecked(map.value(FileHandler::FileAppend, true).toBool());
}

/********************************************************/

void FileHandlerWidget::openFileDialog()
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
