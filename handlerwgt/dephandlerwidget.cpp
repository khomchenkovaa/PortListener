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
    connect(ui->cmbTypeValue, &QComboBox::currentTextChanged,
            this, &DepHandlerWidget::onTypeValueCmbChanged);
    onTypeValueCmbChanged();
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
    map.insert(DepHandler::TypeValue, ui->cmbTypeValue->currentText());
    map.insert(DepHandler::TypeColumn, ui->spinTypeColumn->value());
    map.insert(DepHandler::IndexColumn, ui->spinIndexColumn->value());
    map.insert(DepHandler::KksColumn, ui->spinKksColumn->value());
    map.insert(DepHandler::IidColumn, ui->spinIidColumn->value());
    return map;
}

/********************************************************/

void DepHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editCsvFile->setText(map.value(DepHandler::CsvFileName).toString());
    ui->cmbTypeValue->setCurrentText(map.value(DepHandler::TypeValue, "NONE").toString());
    ui->spinTypeColumn->setValue(map.value(DepHandler::TypeColumn, -1).toInt());
    ui->spinIndexColumn->setValue(map.value(DepHandler::IndexColumn, -1).toInt());
    ui->spinKksColumn->setValue(map.value(DepHandler::KksColumn, -1).toInt());
    ui->spinIidColumn->setValue(map.value(DepHandler::IidColumn, -1).toInt());
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

void DepHandlerWidget::onTypeValueCmbChanged()
{
    if (ui->cmbTypeValue->currentIndex()) {
        ui->lblTypeColumn->setHidden(true);
        ui->spinTypeColumn->setHidden(true);
    } else {
        ui->lblTypeColumn->setHidden(false);
        ui->spinTypeColumn->setHidden(false);
    }
}

/********************************************************/
