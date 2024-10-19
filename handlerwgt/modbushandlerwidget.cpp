#include "modbushandlerwidget.h"
#include "ui_modbushandlerwidget.h"

#include <QFileInfo>
#include <QFileDialog>

/********************************************************/

ModbusHandlerWidget::ModbusHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::ModbusHandlerWidget)
{
    ui->setupUi(this);
    ModbusHandlerWidget::setSettings(SettingsMap());
    connect(ui->btnAoCsvFile, &QAbstractButton::clicked,
            this, &ModbusHandlerWidget::openAoCsvFileDialog);
    connect(ui->btnDoCsvFile, &QAbstractButton::clicked,
            this, &ModbusHandlerWidget::openDoCsvFileDialog);
    connect(ui->btnOutputFile, &QAbstractButton::clicked,
            this, &ModbusHandlerWidget::openOutputFileDialog);
}

/********************************************************/

ModbusHandlerWidget::~ModbusHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap ModbusHandlerWidget::settings() const
{
    SettingsMap map;
    map.insert(ModbusHandler::AoCsvFileName, ui->editAoCsvFile->text());
    map.insert(ModbusHandler::DoCsvFileName, ui->editDoCsvFile->text());
    map.insert(ModbusHandler::OutFileName, ui->editOutputFile->text());
    map.insert(ModbusHandler::FileAppend, ui->chkAppend->isChecked());
    return map;
}

/********************************************************/

void ModbusHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editAoCsvFile->setText(map.value(ModbusHandler::AoCsvFileName).toString());
    ui->editDoCsvFile->setText(map.value(ModbusHandler::DoCsvFileName).toString());
    ui->editOutputFile->setText(map.value(ModbusHandler::OutFileName).toString());
    ui->chkAppend->setChecked(map.value(ModbusHandler::FileAppend, true).toBool());
}

/********************************************************/

void ModbusHandlerWidget::openAoCsvFileDialog()
{
    QString dirPath = QCoreApplication::applicationDirPath();
    QString fileName = ui->editAoCsvFile->text();
    const QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        dirPath = fileInfo.path();
    }

    fileName = QFileDialog::getOpenFileName(this, tr("Choose AO Csv Config File"),
                                            dirPath,
                                            "CSV files (*.csv)");
    if (!fileName.isEmpty()) {
        ui->editAoCsvFile->setText(fileName);
    }
}

/********************************************************/

void ModbusHandlerWidget::openDoCsvFileDialog()
{
    QString dirPath = QCoreApplication::applicationDirPath();
    QString fileName = ui->editDoCsvFile->text();
    const QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        dirPath = fileInfo.path();
    }

    fileName = QFileDialog::getOpenFileName(this, tr("Choose DO Csv Config File"),
                                            dirPath,
                                            "CSV files (*.csv)");
    if (!fileName.isEmpty()) {
        ui->editDoCsvFile->setText(fileName);
    }
}

/********************************************************/

void ModbusHandlerWidget::openOutputFileDialog()
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
