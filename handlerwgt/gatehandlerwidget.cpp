#include "gatehandlerwidget.h"
#include "ui_gatehandlerwidget.h"

#include <QFileInfo>
#include <QFileDialog>

/********************************************************/

GateHandlerWidget::GateHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::GateHandlerWidget)
{
    ui->setupUi(this);
    GateHandlerWidget::setSettings(SettingsMap());
    connect(ui->btnCsvFile, &QAbstractButton::clicked,
            this, &GateHandlerWidget::openCsvFileDialog);
    connect(ui->btnDefFile, &QAbstractButton::clicked,
            this, &GateHandlerWidget::openDefFileDialog);
    connect(ui->btnOutputFile, &QAbstractButton::clicked,
            this, &GateHandlerWidget::openOutputFileDialog);
}

/********************************************************/

GateHandlerWidget::~GateHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap GateHandlerWidget::settings() const
{
    SettingsMap map;
    map.insert(GateHandler::CsvFileName, ui->editCsvFile->text());
    map.insert(GateHandler::DefFileName, ui->editDefFile->text());
    map.insert(GateHandler::OutFileName, ui->editOutputFile->text());
    map.insert(GateHandler::FileAppend, ui->chkAppend->isChecked());
    return map;
}

/********************************************************/

void GateHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editCsvFile->setText(map.value(GateHandler::CsvFileName).toString());
    ui->editDefFile->setText(map.value(GateHandler::DefFileName).toString());
    ui->editOutputFile->setText(map.value(GateHandler::OutFileName).toString());
    ui->chkAppend->setChecked(map.value(GateHandler::FileAppend, true).toBool());
}

/********************************************************/

void GateHandlerWidget::openCsvFileDialog()
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

void GateHandlerWidget::openDefFileDialog()
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

void GateHandlerWidget::openOutputFileDialog()
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
