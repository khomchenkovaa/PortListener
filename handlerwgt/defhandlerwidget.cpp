#include "defhandlerwidget.h"
#include "ui_defhandlerwidget.h"

#include <QFileInfo>
#include <QFileDialog>

/********************************************************/

DefHandlerWidget::DefHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::DefHandlerWidget)
{
    ui->setupUi(this);
    DefHandlerWidget::setSettings(SettingsMap());
    connect(ui->btnDefFile, &QAbstractButton::clicked,
            this, &DefHandlerWidget::openDefFileDialog);
}

/********************************************************/

DefHandlerWidget::~DefHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap DefHandlerWidget::settings() const
{
    SettingsMap map;
    map.insert(DefHandler::DefFileName, ui->editDefFile->text());
    return map;
}

/********************************************************/

void DefHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editDefFile->setText(map.value(DefHandler::DefFileName).toString());
}

/********************************************************/

void DefHandlerWidget::openDefFileDialog()
{
    QString dirPath = QCoreApplication::applicationDirPath();
    QString fileName = ui->btnDefFile->text();
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
