#include "dephandlerwidget.h"
#include "ui_dephandlerwidget.h"

#include <QFileInfo>
#include <QFileDialog>

/********************************************************/

DepHandlerWidget::DepHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::DepHandlerWidget)
{
    ui->setupUi(this);
    DepHandlerWidget::setSettings(SettingsMap());
    connect(ui->btnCsvFile, &QAbstractButton::clicked,
            this, &DepHandlerWidget::openCsvFileDialog);
    connect(ui->btnDefFile, &QAbstractButton::clicked,
            this, &DepHandlerWidget::openDefFileDialog);
    connect(ui->btnOutputFile, &QAbstractButton::clicked,
            this, &DepHandlerWidget::openOutputFileDialog);
}

/********************************************************/

DepHandlerWidget::~DepHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap DepHandlerWidget::settings() const
{
    SettingsMap map;
    map.insert(DepHandler::CsvFileName, ui->editCsvFile->text());
    map.insert(DepHandler::DefFileName, ui->editDefFile->text());
    map.insert(DepHandler::OutFileName, ui->editOutputFile->text());
    map.insert(DepHandler::FileAppend, ui->chkAppend->isChecked());
    return map;
}

/********************************************************/

void DepHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editCsvFile->setText(map.value(DepHandler::CsvFileName).toString());
    ui->editDefFile->setText(map.value(DepHandler::DefFileName).toString());
    ui->editOutputFile->setText(map.value(DepHandler::OutFileName).toString());
    ui->chkAppend->setChecked(map.value(DepHandler::FileAppend, true).toBool());
}

/********************************************************/

void DepHandlerWidget::openCsvFileDialog()
{
    QString dirPath = QCoreApplication::applicationDirPath();
    QString fileName = ui->editCsvFile->text();
    const QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        dirPath = fileInfo.path();
    }

    fileName = QFileDialog::getOpenFileName(this, tr("Choose Csv Config File"),
                                            dirPath,
                                            "CSV files (*.csv)");
    if (!fileName.isEmpty()) {
        ui->editCsvFile->setText(fileName);
    }
}

/********************************************************/

void DepHandlerWidget::openDefFileDialog()
{
    QString dirPath = QCoreApplication::applicationDirPath();
    QString fileName = ui->editDefFile->text();
    const QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        dirPath = fileInfo.path();
    }

    fileName = QFileDialog::getOpenFileName(this, tr("Choose Def Config File"),
                                            dirPath,
                                            "DEF files (*.def)");
    if (!fileName.isEmpty()) {
        ui->editDefFile->setText(fileName);
    }
}

/********************************************************/

void DepHandlerWidget::openOutputFileDialog()
{
    QString dirPath = QCoreApplication::applicationDirPath();
    QString fileName = ui->editOutputFile->text();
    const QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        dirPath = fileInfo.path();
    }

    fileName = QFileDialog::getSaveFileName(this, tr("Choose Output File"), dirPath);
    if (!fileName.isEmpty()) {
        ui->editOutputFile->setText(fileName);
    }
}

/********************************************************/
