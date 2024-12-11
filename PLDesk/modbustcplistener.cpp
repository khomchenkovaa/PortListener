#include "modbustcplistener.h"
#include "ui_modbustcplistener.h"

#include "iodecoder.h"
#include "messagehandlerwgt.h"
#include "modbushandler.h"
#include "modbushandlerconf.h"
#include "modbushandlerwidget.h"

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
    setupUiDefaultState();

    QModbusDataUnitMap reg;
    reg.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, REG_MAX });
    reg.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, REG_MAX });
    reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, REG_MAX });
    reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, REG_MAX });
    m_ModbusDevice.setMap(reg);

    // connection block
    connect(&m_ModbusDevice, &QModbusServer::dataWritten,
            this, &ModbusTcpListener::onDataUpdated);
    connect(&m_ModbusDevice, &QModbusServer::stateChanged,
            this, &ModbusTcpListener::updateStatus);
    connect(&m_ModbusDevice, &QModbusServer::errorOccurred,
            this, &ModbusTcpListener::handleDeviceError);
    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &ModbusTcpListener::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &ModbusTcpListener::doDisconnect);

    setupEditor();

    // action block
    ui->boxAction->setChecked(false);
    connect(ui->boxAction, &QGroupBox::clicked,
            this, &ModbusTcpListener::activateHandler);

    updateStatus();
}

/********************************************************/

ModbusTcpListener::~ModbusTcpListener()
{
    disconnect(this, nullptr, nullptr, nullptr);
    doDisconnect();
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
    if (newError != QModbusDevice::NoError) {
        printError("Modbus TCP error", m_ModbusDevice.errorString());
    }
}

/********************************************************/

QTextBrowser *ModbusTcpListener::textLog() const
{
    return ui->textLog;
}

/********************************************************/

void ModbusTcpListener::doConnect()
{
    quint16 port = ui->spinPort->value();
    quint16 addr = ui->spinServerId->value();
    m_ModbusDevice.setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    m_ModbusDevice.setServerAddress(addr);
    if (!m_ModbusDevice.connectDevice()) {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Modbus TCP Port %1 connection error!\n%2")
                              .arg(port)
                              .arg(m_ModbusDevice.errorString()));
    } else if (initHandler()) {
        connect(handler(), &MessageHandler::logMessage,
                this, &ModbusTcpListener::printMessage);
        connect(handler(), &MessageHandler::logError,
                this, &ModbusTcpListener::printError);
    }
    const auto errors = handlerErrors();
    for (const auto &error : errors) {
        printError(handlerName(), error);
    }
    clearErrors();
    updateStatus();
}

/********************************************************/

void ModbusTcpListener::doDisconnect()
{
    if (m_ModbusDevice.state() == QModbusDevice::ConnectedState) {
        m_ModbusDevice.disconnectDevice();
    }
    disconnectHandler();
    updateStatus();
}

/********************************************************/

void ModbusTcpListener::activateHandler()
{
    if (d.editor) {
        d.editor->deleteLater();
        d.editor = Q_NULLPTR;
    }
    if (d.handler) {
        d.handler->deleteLater();
        d.handler = Q_NULLPTR;
    }

    if (ui->boxAction->isChecked()) {
        d.handler = new ModbusHandler(this);
        d.editor  = new ModbusHandlerWidget(this);
    }

    if (d.editor) {
        ui->boxAction->layout()->addWidget(d.editor);
    }
}

/********************************************************/

void ModbusTcpListener::updateEditorStatus()
{
    switch(ui->cmbTable->currentData().toInt()) {
    case QModbusDataUnit::Coils:
    case QModbusDataUnit::DiscreteInputs:
        if (ui->cmbValueType->count() != 1) {
            QSignalBlocker block(ui->cmbValueType);
            ui->cmbValueType->clear();
            ui->cmbValueType->addItem(tr("Binary"), Modbus::BinaryType);
            ui->cmbValueType->setCurrentIndex(0);
            ui->editValue->setVisible(false);
            ui->chkValue->setVisible(true);
        }
        break;
    case QModbusDataUnit::InputRegisters:
    case QModbusDataUnit::HoldingRegisters:
        if (ui->cmbValueType->count() != 3) {
            QSignalBlocker block(ui->cmbValueType);
            ui->cmbValueType->clear();
            ui->cmbValueType->addItem(tr("Real (4-byte)"), Modbus::RealType);
            ui->cmbValueType->addItem(tr("Double Int (4-byte)"), Modbus::DWordType);
            ui->cmbValueType->addItem(tr("Integer (2-byte)"), Modbus::IntType);
            ui->cmbValueType->setCurrentIndex(0);
            ui->chkValue->setVisible(false);
            ui->editValue->setVisible(true);
            ui->editValue->clear();
        }
        break;
    }
    doReadValue();
}

/********************************************************/

void ModbusTcpListener::doReadValue()
{
    auto regType = static_cast<QModbusDataUnit::RegisterType>(ui->cmbTable->currentData().toInt());
    auto addr    = ui->spinAddress->value();

    quint16 value = 0;
    m_ModbusDevice.data(regType, quint16(addr), &value);

    switch (ui->cmbValueType->currentData().toInt()) {
    case Modbus::BinaryType:
        if (regType == QModbusDataUnit::Coils ||
                regType == QModbusDataUnit::DiscreteInputs) {
            ui->chkValue->setChecked(value);
        }
        break;
    case Modbus::RealType:
        if (regType == QModbusDataUnit::InputRegisters ||
                regType == QModbusDataUnit::HoldingRegisters) {
            quint16 value2 = 0;
            m_ModbusDevice.data(regType, quint16(addr+1), &value2);
            Modbus::ModbusValue v;
            v.in.first = value;
            v.in.last  = value2;
            ui->editValue->setText(QString::number(v.outFloat, 'f'));
        }
        break;
    case Modbus::DWordType:
        if (regType == QModbusDataUnit::InputRegisters ||
                regType == QModbusDataUnit::HoldingRegisters) {
            quint16 value2 = 0;
            m_ModbusDevice.data(regType, quint16(addr+1), &value2);
            Modbus::ModbusValue v;
            v.in.first = value;
            v.in.last  = value2;
            ui->editValue->setText(QString::number(v.outUInt));
        }
        break;
    case Modbus::IntType:
        if (regType == QModbusDataUnit::InputRegisters ||
                regType == QModbusDataUnit::HoldingRegisters) {
            ui->editValue->setText(QString::number(value));
        }
        break;
    }
}

/********************************************************/

void ModbusTcpListener::doWriteValue()
{
    auto regType = static_cast<QModbusDataUnit::RegisterType>(ui->cmbTable->currentData().toInt());
    auto addr    = ui->spinAddress->value();

    quint16 value = 0;
    m_ModbusDevice.data(regType, quint16(addr), &value);

    switch (ui->cmbValueType->currentData().toInt()) {
    case Modbus::BinaryType:
        if (regType == QModbusDataUnit::Coils ||
                regType == QModbusDataUnit::DiscreteInputs) {
            quint16 value = ui->chkValue->isChecked();
            m_ModbusDevice.setData(regType, quint16(addr), value);
        }
        break;
    case Modbus::RealType:
        if (regType == QModbusDataUnit::InputRegisters ||
                regType == QModbusDataUnit::HoldingRegisters) {
            Modbus::ModbusValue v;
            v.outFloat = ui->editValue->text().toDouble();
            m_ModbusDevice.setData(regType, quint16(addr), v.in.first);
            m_ModbusDevice.setData(regType, quint16(addr+1), v.in.last);
        }
        break;
    case Modbus::DWordType:
        if (regType == QModbusDataUnit::InputRegisters ||
                regType == QModbusDataUnit::HoldingRegisters) {
            Modbus::ModbusValue v;
            v.outUInt = ui->editValue->text().toUInt();
            m_ModbusDevice.setData(regType, quint16(addr), v.in.first);
            m_ModbusDevice.setData(regType, quint16(addr+1), v.in.last);
        }
        break;
    case Modbus::IntType:
        if (regType == QModbusDataUnit::InputRegisters ||
                regType == QModbusDataUnit::HoldingRegisters) {
            quint16 value = ui->editValue->text().toUInt();
            m_ModbusDevice.setData(regType, quint16(addr), value);
        }
        break;
    }
}

/********************************************************/

void ModbusTcpListener::setupUiDefaultState()
{
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

/********************************************************/

void ModbusTcpListener::setupEditor()
{
    ui->cmbTable->addItem(tr("Coils"), QModbusDataUnit::Coils);
    ui->cmbTable->addItem(tr("Discrete Inputs"), QModbusDataUnit::DiscreteInputs);
    ui->cmbTable->addItem(tr("Input Registers"), QModbusDataUnit::InputRegisters);
    ui->cmbTable->addItem(tr("Holding Registers"), QModbusDataUnit::HoldingRegisters);
    ui->cmbTable->setCurrentIndex(ui->cmbTable->findData(QModbusDataUnit::HoldingRegisters));

    ui->spinAddress->setMaximum(REG_MAX);

    connect(ui->cmbTable, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusTcpListener::updateEditorStatus);
    connect(ui->btnValueRead, &QAbstractButton::clicked,
            this, &ModbusTcpListener::doReadValue);
    connect(ui->spinAddress, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ModbusTcpListener::doReadValue);
    connect(ui->cmbValueType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ModbusTcpListener::doReadValue);
    connect(ui->btnValueWrite, &QAbstractButton::clicked,
            this, &ModbusTcpListener::doWriteValue);

    updateEditorStatus();
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
        emit tabText(QString("Modbus TCP [%1:%2]").arg(ui->spinPort->value()).arg(ui->spinServerId->value()));
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
    // check if address is not even
    if (address % 2 == 1) {
        address--;
        size++;
    }

    // check size shout be not even too
    if (size % 2 == 1) size++;

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
    auto host = "Coils";
    auto info = QString("from %1 size %2").arg(address).arg(size);
    printInfo(host, info);

    auto msg = PMessage::create();
    msg->payload = data;
    msg->payloadType = QMetaType::QBitArray;
    msg->headers.insert("address", address);
    msg->headers.insert("size", size);
    doHandle(msg);
}

/********************************************************/

void ModbusTcpListener::processHoldingRegisters(int address, int size, const QVariantList &data)
{
    auto host = "Holding registers";
    auto info = QString("from %1 size %2").arg(address).arg(size);
    printInfo(host, info);

    auto msg = PMessage::create();
    msg->payload = data;
    msg->payloadType = QMetaType::QVariantList;
    msg->headers.insert("address", address);
    msg->headers.insert("size", size);
    doHandle(msg);
}

/********************************************************/
