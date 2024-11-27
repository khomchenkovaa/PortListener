#include "modbustcpclient.h"
#include "ui_modbustcpclient.h"

#include "messagehandlerwgt.h"

#include <QMessageBox>


/********************************************************/

ModbusTcpClient::ModbusTcpClient(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::ModbusTcpClient)
{
    ui->setupUi(this);

    ui->cmbTable->addItem(tr("Coils"), QModbusDataUnit::Coils);
    ui->cmbTable->addItem(tr("Discrete Inputs"), QModbusDataUnit::DiscreteInputs);
    ui->cmbTable->addItem(tr("Input Registers"), QModbusDataUnit::InputRegisters);
    ui->cmbTable->addItem(tr("Holding Registers"), QModbusDataUnit::HoldingRegisters);
    ui->cmbTable->setCurrentIndex(ui->cmbTable->findData(QModbusDataUnit::HoldingRegisters));

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
    QString host = ui->editHost->text();
    quint16 port = ui->spinPort->value();
    m_ModbusDevice.setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    m_ModbusDevice.setConnectionParameter(QModbusDevice::NetworkAddressParameter, host);
    m_ModbusDevice.setTimeout(1);
    m_ModbusDevice.setNumberOfRetries(3);
    if (!m_ModbusDevice.connectDevice()) {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Modbus client %1:%2 connection error!\n%3")
                              .arg(host)
                              .arg(port)
                              .arg(m_ModbusDevice.errorString()));
    }
    if (m_ModbusDevice.state() == QModbusDevice::ConnectedState) {
        initHandler(true);
        connect(handler(), &MessageHandler::logMessage,
                this, &ModbusTcpClient::printMessage);
        connect(handler(), &MessageHandler::logError,
                this, &ModbusTcpClient::printError);
    }
    const auto errors = handlerErrors();
    for (const auto &error : errors) {
        printError(handlerName(), error);
    }
    clearErrors();
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
