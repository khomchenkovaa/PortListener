#include "sockhandlerwidget.h"
#include "ui_sockhandlerwidget.h"

/********************************************************/

SockHandlerWidget::SockHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::SockHandlerWidget)
{
    ui->setupUi(this);
    setSettings(SettingsMap());
}

/********************************************************/

SockHandlerWidget::~SockHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap SockHandlerWidget::settings() const
{
    SettingsMap map;
    map.insert(SockHandler::Socket, ui->editSocket->text());
    return map;
}

/********************************************************/

void SockHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editSocket->setText(map.value(SockHandler::Socket).toString());
}

/********************************************************/
