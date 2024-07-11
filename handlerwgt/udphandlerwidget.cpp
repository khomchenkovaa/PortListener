#include "udphandlerwidget.h"
#include "ui_udphandlerwidget.h"

/********************************************************/

UdpHandlerWidget::UdpHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::UdpHandlerWidget)
{
    ui->setupUi(this);
    UdpHandlerWidget::setSettings(SettingsMap());
}

/********************************************************/

UdpHandlerWidget::~UdpHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap UdpHandlerWidget::settings() const
{
    SettingsMap map;
    map.insert(UdpHandler::Host, ui->editHost->text());
    map.insert(UdpHandler::Port, ui->spinPort->value());
    return map;
}

/********************************************************/

void UdpHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editHost->setText(map.value(UdpHandler::Host, "localhost").toString());
    ui->spinPort->setValue(map.value(UdpHandler::Port, 2424).toInt());
}

/********************************************************/
