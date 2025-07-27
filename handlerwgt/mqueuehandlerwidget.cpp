#include "mqueuehandlerwidget.h"
#include "ui_mqueuehandlerwidget.h"

/********************************************************/

MqueueHandlerWidget::MqueueHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::MqueueHandlerWidget)
{
    ui->setupUi(this);
    MqueueHandlerWidget::setSettings(SettingsMap());
}

/********************************************************/

MqueueHandlerWidget::~MqueueHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap MqueueHandlerWidget::settings() const
{
    SettingsMap map;
    map.insert(MqueueHandler::Mqueue, ui->editMqueue->text());
    return map;
}

/********************************************************/

void MqueueHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editMqueue->setText(map.value(MqueueHandler::Mqueue).toString());
}

/********************************************************/
