#include "modbustcplistener.h"
#include "ui_modbustcplistener.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"

#include <QHostAddress>
#include <QTcpSocket>
#include <QTextCodec>
#include <QMessageBox>

#include <QDebug>

/********************************************************/

ModbusTcpListener::ModbusTcpListener(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::ModbusTcpListener),
    m_ModbusDevice(this)
{
    ui->setupUi(this);

    QModbusDataUnitMap reg;
    reg.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, REG_MAX });
    reg.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, REG_MAX });
    reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, REG_MAX });
    reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, REG_MAX });
    m_ModbusDevice.setMap(reg);

    connect(&m_ModbusDevice, &QModbusServer::dataWritten,
            this, &ModbusTcpListener::onDataUpdated);
    connect(&m_ModbusDevice, &QModbusServer::stateChanged,
            this, &ModbusTcpListener::updateStatus);
    connect(&m_ModbusDevice, &QModbusServer::errorOccurred,
            this, &ModbusTcpListener::handleDeviceError);


    ui->cmbHandler->addItems(handlers());

    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &ModbusTcpListener::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &ModbusTcpListener::doDisconnect);
    connect(ui->cmbHandler, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusTcpListener::changeHandler);
    // configure UI default state
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);

    updateStatus();
}

/********************************************************/

ModbusTcpListener::~ModbusTcpListener()
{
    if (m_ModbusDevice.state() == QModbusDevice::ConnectedState) {
        m_ModbusDevice.disconnectDevice();
    }
    delete ui;
}

/********************************************************/

void ModbusTcpListener::onDataUpdated(QModbusDataUnit::RegisterType table, int address, int size)
{
    switch (table) {
    case QModbusDataUnit::Coils:
        onCoilsUpdated(address, size);
        break;
    case QModbusDataUnit::HoldingRegisters:
        onHoldingRegistersUpdated(address, size);
        break;
    default:
        break;
    }
}

/********************************************************/

void ModbusTcpListener::handleDeviceError(QModbusDevice::Error newError)
{
    if (newError == QModbusDevice::NoError)
        return;
    ui->textLog->append(QString("<font color=\"black\">%1 -> </font><font color=\"red\">%2</font>")
                        .arg("Modbus TCP error", m_ModbusDevice.errorString()));

}

/********************************************************/

void ModbusTcpListener::doConnect()
{
    quint16 port = ui->spinPort->value();
    quint16 addr = ui->spinServerAddress->value();
    m_ModbusDevice.setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    m_ModbusDevice.setServerAddress(addr);
    if (!m_ModbusDevice.connectDevice()) {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Modbus TCP Port %1 connection error!\n%2")
                              .arg(port)
                              .arg(m_ModbusDevice.errorString()));
    }
    if (m_ModbusDevice.state() == QModbusDevice::ConnectedState) {
        initHandler(true);
    }
    const auto errors = handlerErrors();
    for (const auto &error : errors) {
        ui->textLog->append(QString("<font color=\"black\">%1 -> </font><font color=\"red\">%2</font>")
                            .arg(handlerName(), error));
    }
    updateStatus();
}

/********************************************************/

void ModbusTcpListener::doDisconnect()
{
    m_ModbusDevice.disconnectDevice();
    disconnectHandler();
    updateStatus();
}

/********************************************************/

void ModbusTcpListener::changeHandler(int index)
{
    auto editor = updateHandler(index);
    if (editor) {
        ui->boxAction->layout()->addWidget(editor);
    }
}

/********************************************************/

void ModbusTcpListener::updateStatus()
{
    if (m_ModbusDevice.state() == QModbusDevice::ConnectedState) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Listening the TCP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("Modbus TCP [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">Choose TCP port to listen</font>"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxAction->setEnabled(true);
        emit tabText(QString("Modbus TCP [-]"));
    }
}

/********************************************************/

void ModbusTcpListener::onCoilsUpdated(int address, int size)
{
    QBitArray bits(size);
    for (int i = 0; i < size; ++i) {
        quint16 value = 0;
        m_ModbusDevice.data(QModbusDataUnit::Coils, quint16(address + i), &value);
        if (value) {
            bits.setBit(i);
        }
    }
    processCoils(address, size, bits);
}

/********************************************************/

void ModbusTcpListener::onHoldingRegistersUpdated(int address, int size)
{
    QVariantList regs;
    for (int i = 0; i < size; ++i) {
        quint16 value = 0;
        m_ModbusDevice.data(QModbusDataUnit::HoldingRegisters, quint16(address + i), &value);
        regs << value;
    }
    processHoldingRegisters(address, size, regs);
}

/********************************************************/

void ModbusTcpListener::processCoils(int address, int size, const QBitArray &data)
{
    auto msg = PMessage::create();
    msg->logger = ui->textLog;
    msg->payload = data;
    msg->payloadType = QMetaType::QBitArray;
    msg->headers.insert("address", address);
    msg->headers.insert("size", size);
    doHandle(msg);

    auto host = QString("Coils from %1 size %2").arg(address).arg(size);
    const auto errors = handlerErrors();
    for (const auto &error : errors) {
        ui->textLog->append(QString("<font color=\"black\">%1 -> </font><font color=\"red\">%2</font>")
                            .arg(host, error));
    }
}

/********************************************************/

void ModbusTcpListener::processHoldingRegisters(int address, int size, const QVariantList &data)
{
    auto msg = PMessage::create();
    msg->logger = ui->textLog;
    msg->payload = data;
    msg->payloadType = QMetaType::QVariantList;
    msg->headers.insert("address", address);
    msg->headers.insert("size", size);
    doHandle(msg);

    auto host = QString("Holding registers from %1 size %2").arg(address).arg(size);
    const auto errors = handlerErrors();
    for (const auto &error : errors) {
        ui->textLog->append(QString("<font color=\"black\">%1 -> </font><font color=\"red\">%2</font>")
                            .arg(host, error));
    }
}

/********************************************************/
