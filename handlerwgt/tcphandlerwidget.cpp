#include "tcphandlerwidget.h"
#include "ui_tcphandlerwidget.h"

/********************************************************/

TcpHandlerWidget::TcpHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::TcpHandlerWidget)
{
    ui->setupUi(this);
    TcpHandlerWidget::setSettings(SettingsMap());
}

/********************************************************/

TcpHandlerWidget::~TcpHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap TcpHandlerWidget::settings() const
{
    SettingsMap map;
    map.insert(TcpHandler::Host, ui->editHost->text());
    map.insert(TcpHandler::Port, ui->spinPort->value());
    return map;
}

/********************************************************/

void TcpHandlerWidget::setSettings(const SettingsMap &map)
{
    ui->editHost->setText(map.value(TcpHandler::Host, "localhost").toString());
    ui->spinPort->setValue(map.value(TcpHandler::Port, 2424).toInt());
}

/********************************************************/
