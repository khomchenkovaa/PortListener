#include "modbustcpclient.h"
#include "ui_modbustcpclient.h"

#include "messagehandlerwgt.h"


/********************************************************/

ModbusTcpClient::ModbusTcpClient(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::ModbusTcpClient)
{
    ui->setupUi(this);

    ui->cmbHandler->addItems(handlers());

    connect(&m_ModbusDevice, &QModbusClient::stateChanged,
            this, &ModbusTcpClient::updateStatus);
    connect(&m_ModbusDevice, &QModbusClient::errorOccurred,
            this, &ModbusTcpClient::handleDeviceError);

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &ModbusTcpClient::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &ModbusTcpClient::doDisconnect);
    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusTcpClient::changeHandler);

    setupUiDefaultState();

    updateStatus();
}

/********************************************************/

ModbusTcpClient::~ModbusTcpClient()
{
    delete ui;
}

/********************************************************/

void ModbusTcpClient::handleDeviceError(QModbusDevice::Error newError)
{
    if (newError != QModbusDevice::NoError) {
        printError("Modbus TCP error", m_ModbusDevice.errorString());
    }
}

/********************************************************/

void ModbusTcpClient::doConnect()
{
    // TODO
    updateStatus();
}

/********************************************************/

void ModbusTcpClient::doDisconnect()
{
    m_ModbusDevice.disconnectDevice();
    disconnectHandler();
    updateStatus();
}

/********************************************************/

void ModbusTcpClient::changeHandler(int index)
{
    auto editor = updateHandler(index);
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

/********************************************************/

void ModbusTcpClient::printInfo(const QString &host, const QString &msg)
{
    ui->textLog->append(QString("<font color=\"black\">%1 -> </font><font color=\"darkblue\">%2</font>")
                        .arg(host, msg));
}

/********************************************************/

void ModbusTcpClient::printMessage(const QString &host, const QString &msg)
{
    ui->textLog->append(QString("<font color=\"black\">%1 -> </font><font color=\"darkgreen\">%2</font>")
                        .arg(host, msg));
}

/********************************************************/

void ModbusTcpClient::printError(const QString &host, const QString &msg)
{
    ui->textLog->append(QString("<font color=\"black\">%1 -> </font><font color=\"red\">%2</font>")
                        .arg(host, msg));
}

/********************************************************/

void ModbusTcpClient::setupUiDefaultState()
{
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

/********************************************************/

void ModbusTcpClient::updateStatus()
{
    if (m_ModbusDevice.state() == QModbusDevice::ConnectedState) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Connected to Modbus TCP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("Modbus Client [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">Choose TCP port to connect</font>"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxAction->setEnabled(true);
        emit tabText(QString("Modbus Client [-]"));
    }
}

/********************************************************/
