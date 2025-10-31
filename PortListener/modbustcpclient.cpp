#include "modbustcpclient.h"
#include "ui_modbustcpclient.h"

#include "messagehandlerwgt.h"
#include "filehandler.h"
#include "filehandlerwidget.h"

#include <QMessageBox>
#include <QTimer>
#include <QFileInfo>
#include <QFileDialog>

/********************************************************/

inline QString takeHost(const QString &sensor, const QVector<quint16> &vars) {
    switch (vars.size()) {
    case 1:
        return QString("%1 [%2]").arg(sensor)
                .arg(vars.constFirst(), 4, 16, QLatin1Char('0'));
    case 2:
        return QString("%1 [%2 %3]").arg(sensor)
                .arg(vars.constFirst(), 4, 16, QLatin1Char('0'))
                .arg(vars.constLast(), 4, 16, QLatin1Char('0'));
    }
    return sensor;
}

/********************************************************/

ModbusTcpClient::ModbusTcpClient(QWidget *parent) :
    ListenerWidget(parent),
    ui(new Ui::ModbusTcpClient)
{
    ui->setupUi(this);
    setupUiDefaultState();

    // connection block
    connect(&m.modbusDevice, &QModbusClient::stateChanged,
            this, &ModbusTcpClient::updateStatus);
    connect(&m.modbusDevice, &QModbusClient::errorOccurred,
            this, &ModbusTcpClient::handleDeviceError);
    connect(ui->btnConnect, &QAbstractButton::clicked,
            this, &ModbusTcpClient::doConnect);
    connect(ui->btnDisconnect, &QAbstractButton::clicked,
            this, &ModbusTcpClient::doDisconnect);

    setupSingleRequestBlock();
    setupCycleRequestBlock();

    // action block
    ui->boxAction->setChecked(false);
    connect(ui->boxAction, &QGroupBox::clicked,
            this, &ModbusTcpClient::activateHandler);

    updateStatus();
}

/********************************************************/

ModbusTcpClient::~ModbusTcpClient()
{
    disconnect(this, nullptr, nullptr, nullptr);
    delete ui;
}

/********************************************************/

void ModbusTcpClient::doReadRequest()
{
    QString host = QString("%1:%2").arg(ui->editHost->text()).arg(ui->spinPort->value());
    if (m.modbusDevice.state() != QModbusDevice::ConnectedState) {
        printError(host, tr("There is no active connection to send request"));
        return;
    }
    const auto table =
            static_cast<QModbusDataUnit::RegisterType>(ui->cmbRegister->currentData().toInt());
    const auto valueType =
            static_cast<Modbus::DataType>(ui->cmbValueType->currentData().toInt());
    int serverId = 0xff;
    int address  = ui->spinAddress->value();
    int interval = Modbus::dataTypeSizeOf(valueType);

    QModbusDataUnit request(table, address, interval);
    if (auto *response = m.modbusDevice.sendReadRequest(request, serverId)) {
        if (!response->isFinished())
            connect(response, &QModbusReply::finished, this, [this, address, valueType](){
                auto reply = qobject_cast<QModbusReply *>(sender());
                if (!reply) return;

                if (reply->error() == QModbusDevice::NoError) {
                    const QModbusDataUnit unit = reply->result();
                    switch(valueType) {
                    case Modbus::RealType: {
                        auto arg = QVector<quint16>() << unit.value(0) << unit.value(1);
                        auto val = Modbus::takeFloat(arg);
                        auto host = takeHost(QString::number(address), arg);
                        auto info = QString::number(val, 'f', 3);
                        ui->editValue->setText(info);
                        printMessage(host, info);
                    } break;
                    case Modbus::DWordType: {
                        auto arg = QVector<quint16>() << unit.value(0) << unit.value(1);
                        auto val = Modbus::takeUInt(arg);
                        auto host = takeHost(QString::number(address), arg);
                        auto info = QString::number(val);
                        ui->editValue->setText(info);
                        printMessage(host, info);
                    } break;
                    case Modbus::IntType: {
                        quint16 val = unit.value(0);
                        auto host = takeHost(QString::number(address), QVector<quint16>() << val);
                        auto info = QString::number(val);
                        ui->editValue->setText(info);
                        printMessage(host, info);
                    } break;
                    case Modbus::BinaryType: {
                        quint16 val = unit.value(0);
                        auto host = takeHost(QString::number(address), QVector<quint16>() << val);
                        ui->chkValue->setChecked(val);
                        printMessage(host, val ? "on" : "off");
                    } break;
                    default:
                        break;
                    }
                } else if (reply->error() == QModbusDevice::ProtocolError) {
                    printError(QString::number(address), tr("Read response error: %1 (Mobus exception: 0x%2)").
                                        arg(reply->errorString()).
                                        arg(reply->rawResult().exceptionCode(), -1, 16));
                } else {
                    printError(QString::number(address), tr("Read response error: %1 (code: 0x%2)").
                                        arg(reply->errorString()).
                                        arg(reply->error(), -1, 16));
                }

                reply->deleteLater();
            });
        else
            delete response; // broadcast replies return immediately
    } else {
        printError(host, m.modbusDevice.errorString());
    }
}

/********************************************************/

void ModbusTcpClient::doWriteRequest()
{
    QString host = QString("%1:%2").arg(ui->editHost->text()).arg(ui->spinPort->value());
    if (m.modbusDevice.state() != QModbusDevice::ConnectedState) {
        printError(host, tr("There is no active connection to send request"));
        return;
    }

    const auto regType =
            static_cast<QModbusDataUnit::RegisterType>(ui->cmbRegister->currentData().toInt());
    const auto valueType =
            static_cast<Modbus::DataType>(ui->cmbValueType->currentData().toInt());
    int serverId = 0xff;
    int address  = ui->spinAddress->value();

    QVector<quint16> data;
    QStringList displayData;
    switch (valueType) {
    case Modbus::BinaryType:
        if (regType == QModbusDataUnit::Coils ||
                regType == QModbusDataUnit::DiscreteInputs) {
            quint16 value = ui->chkValue->isChecked();
            data << value;
            displayData << (value ? "1" : "0");
        }
        break;
    case Modbus::RealType:
        if (regType == QModbusDataUnit::InputRegisters ||
                regType == QModbusDataUnit::HoldingRegisters) {
            Modbus::ModbusValue v;
            v.outFloat = ui->editValue->text().toDouble();
            data << v.in.first << v.in.last;
            displayData << QString("0x%1").arg(v.in.first, 4, 16, QLatin1Char('0')) << QString("0x%1").arg(v.in.last, 4, 16, QLatin1Char('0'));
        }
        break;
    case Modbus::DWordType:
        if (regType == QModbusDataUnit::InputRegisters ||
                regType == QModbusDataUnit::HoldingRegisters) {
            Modbus::ModbusValue v;
            v.outUInt = ui->editValue->text().toUInt();
            data << v.in.first << v.in.last;
            displayData << QString("0x%1").arg(v.in.first, 4, 16, QLatin1Char('0')) << QString("0x%1").arg(v.in.last, 4, 16, QLatin1Char('0'));
        }
        break;
    case Modbus::IntType:
        if (regType == QModbusDataUnit::InputRegisters ||
                regType == QModbusDataUnit::HoldingRegisters) {
            quint16 value = ui->editValue->text().toUInt();
            data << value;
            displayData << QString("0x%1").arg(value, 4, 16, QLatin1Char('0'));
        }
        break;
    default:
        break;
    }

    QModbusDataUnit request(regType, address, data);
    if (auto *response = m.modbusDevice.sendWriteRequest(request, serverId)) {
        if (!response->isFinished())
            connect(response, &QModbusReply::finished, this, [this, address, displayData](){
                auto reply = qobject_cast<QModbusReply *>(sender());
                if (!reply) return;
                const QString serverAddress = QString::number(address);

                if (reply->error() == QModbusDevice::NoError) {
                    printMessage(serverAddress, tr("Write request done [ %1 ]").arg(displayData.join(",")));

                } else if (reply->error() == QModbusDevice::ProtocolError) {
                    printError(serverAddress, tr("Write response error: %1 (Mobus exception: 0x%2)").
                                        arg(reply->errorString()).
                                        arg(reply->rawResult().exceptionCode(), -1, 16));
                } else {
                    printError(serverAddress, tr("Write response error: %1 (code: 0x%2)").
                                        arg(reply->errorString()).
                                        arg(reply->error(), -1, 16));
                }

                reply->deleteLater();
            });
        else
            delete response; // broadcast replies return immediately
    } else {
        printError(host, m.modbusDevice.errorString());
    }

}

/********************************************************/

void ModbusTcpClient::doModbusRequest()
{
    if (m.modbusDevice.state() == QModbusDevice::ConnectedState) {
        QString host = QString("%1:%2").arg(ui->editHost->text()).arg(ui->spinPort->value());
        printInfo(host, "Start request cycle");

        auto time = QTime::currentTime().msecsSinceStartOfDay() / 1000;
        printMessage(host, QString::number(time));
        doHandle(QString("%1\n").arg(time));

        const auto table =
                static_cast<QModbusDataUnit::RegisterType>(ui->cmbTable->currentData().toInt());
        int serverId = 0xff;

        for (const auto &item : qAsConst(m.conf.items)) {
            QModbusDataUnit request(table, item.ad, Modbus::dataTypeSizeOf(item.dt));
            if (auto *response = m.modbusDevice.sendReadRequest(request, serverId)) {
                if (!response->isFinished())
                    connect(response, &QModbusReply::finished, this, [this, item](){
                        auto reply = qobject_cast<QModbusReply *>(sender());
                        if (!reply) return;

                        if (reply->error() == QModbusDevice::NoError) {
                            const QModbusDataUnit unit = reply->result();
                            switch(item.dt) {
                            case Modbus::RealType: {
                                auto arg = QVector<quint16>() << unit.value(0) << unit.value(1);
                                auto val = Modbus::takeFloat(arg);
                                auto host = takeHost(item.pin, arg);
                                auto info = QString::number(val, 'f', 3);
                                printReply(host, info);
                                doHandle(QString("%1;%2\n").arg(item.pin, info));
                            } break;
                            case Modbus::DWordType: {
                                auto arg = QVector<quint16>() << unit.value(0) << unit.value(1);
                                auto val = Modbus::takeUInt(arg);
                                auto host = takeHost(item.pin, arg);
                                auto info = QString::number(val);
                                printReply(host, info);
                                doHandle(QString("%1;%2\n").arg(item.pin, info));
                            } break;
                            case Modbus::IntType: {
                                quint16 val = unit.value(0);
                                auto host = takeHost(item.pin, QVector<quint16>() << val);
                                auto info = QString::number(val);
                                printReply(host, info);
                                doHandle(QString("%1;%2\n").arg(item.pin, info));
                            } break;
                            default:
                                break;
                            }
                        } else if (reply->error() == QModbusDevice::ProtocolError) {
                            printError(item.pin, tr("Read response error: %1 (Mobus exception: 0x%2)").
                                                arg(reply->errorString()).
                                                arg(reply->rawResult().exceptionCode(), -1, 16));
                        } else {
                            printError(item.pin, tr("Read response error: %1 (code: 0x%2)").
                                                arg(reply->errorString()).
                                                arg(reply->error(), -1, 16));
                        }

                        reply->deleteLater();
                    });
                else
                    delete response; // broadcast replies return immediately
            } else {
                printError(item.pin, m.modbusDevice.errorString());
            }
        }
    }

    m.timer.start(); // singleShort
}

/********************************************************/

void ModbusTcpClient::handleDeviceError(QModbusDevice::Error newError)
{
    if (newError != QModbusDevice::NoError) {
        printError("Modbus TCP error", m.modbusDevice.errorString());
    }
}

/********************************************************/

QTextBrowser *ModbusTcpClient::textLog() const
{
    return ui->textLog;
}

/********************************************************/

void ModbusTcpClient::doConnect()
{
    QString host = ui->editHost->text();
    quint16 port = ui->spinPort->value();
    m.modbusDevice.setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    m.modbusDevice.setConnectionParameter(QModbusDevice::NetworkAddressParameter, host);
    m.modbusDevice.setTimeout(1);
    m.modbusDevice.setNumberOfRetries(3);
    if (!m.modbusDevice.connectDevice()) {
        QMessageBox::critical(this, QApplication::applicationDisplayName(),
                              tr("Modbus client %1:%2 connection error!\n%3")
                              .arg(host)
                              .arg(port)
                              .arg(m.modbusDevice.errorString()));
    }

    if (initHandler()) {
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
    m.modbusDevice.disconnectDevice();
    disconnectHandler();
    updateStatus();
}

/********************************************************/

void ModbusTcpClient::activateHandler()
{
    if (d.actionEditor) {
        d.actionEditor->deleteLater();
        d.actionEditor = Q_NULLPTR;
    }
    if (d.actionHandler) {
        d.actionHandler->deleteLater();
        d.actionHandler = Q_NULLPTR;
    }

    if (ui->boxAction->isChecked()) {
        d.actionHandler = new FileHandler(this);
        d.actionEditor  = new FileHandlerWidget(this);
    }

    if (d.actionEditor) {
        ui->boxAction->layout()->addWidget(d.actionEditor);
    }
}

/********************************************************/

void ModbusTcpClient::onReadReady()
{

}

/********************************************************/

void ModbusTcpClient::setupUiDefaultState()
{
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);
}

/********************************************************/

void ModbusTcpClient::setupSingleRequestBlock()
{
    ui->cmbRegister->addItem(tr("Coils"), QModbusDataUnit::Coils);
    ui->cmbRegister->addItem(tr("Discrete Inputs"), QModbusDataUnit::DiscreteInputs);
    ui->cmbRegister->addItem(tr("Input Registers"), QModbusDataUnit::InputRegisters);
    ui->cmbRegister->addItem(tr("Holding Registers"), QModbusDataUnit::HoldingRegisters);

    ui->spinAddress->setMaximum(REG_MAX);

    connect(ui->cmbRegister, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](){
        switch(ui->cmbRegister->currentData().toInt()) {
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
    });

    ui->cmbRegister->setCurrentIndex(ui->cmbRegister->findData(QModbusDataUnit::HoldingRegisters));

    connect(ui->cmbValueType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](){
        ui->editValue->clear();
    });

    connect(ui->btnValueRead, &QAbstractButton::clicked,
            this, &ModbusTcpClient::doReadRequest);
    connect(ui->btnValueWrite, &QAbstractButton::clicked,
            this, &ModbusTcpClient::doWriteRequest);
}

/********************************************************/

void ModbusTcpClient::setupCycleRequestBlock()
{
    ui->cmbTable->addItem(tr("Coils"), QModbusDataUnit::Coils);
    ui->cmbTable->addItem(tr("Discrete Inputs"), QModbusDataUnit::DiscreteInputs);
    ui->cmbTable->addItem(tr("Input Registers"), QModbusDataUnit::InputRegisters);
    ui->cmbTable->addItem(tr("Holding Registers"), QModbusDataUnit::HoldingRegisters);
    ui->cmbTable->setCurrentIndex(ui->cmbTable->findData(QModbusDataUnit::HoldingRegisters));

    connect(ui->btnConfig, &QAbstractButton::clicked, this, [this](){
        const QFileInfo fileInfo(ui->editConfig->text());
        const QString dirPath = fileInfo.exists() ? fileInfo.path() : QCoreApplication::applicationDirPath();
        QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Sig Config File"),
                                                dirPath,
                                                "SIG files (*.sig)");
        if (!fileName.isEmpty()) {
            ui->editConfig->setText(fileName);
        }
    });

    m.timer.setSingleShot(true);
    connect(&m.timer, &QTimer::timeout,
            this, &ModbusTcpClient::doModbusRequest);

    connect(ui->btnStart, &QAbstractButton::clicked, this, [this](){
        QString sigFile = ui->editConfig->text().trimmed();
        if (sigFile.isEmpty()) sigFile = ":/resources/scrload/scrload.sig";
        m.conf.items.clear();
        m.conf.load(sigFile);
        int msec = ui->spinFrequency->value() * 1000;
        m.timer.setInterval(msec);
        m.timer.start();
    });
    connect(ui->btnStop, &QAbstractButton::clicked, this, [this](){
        m.timer.stop();
    });
}

/********************************************************/

void ModbusTcpClient::updateStatus()
{
    if (m.modbusDevice.state() == QModbusDevice::ConnectedState) {
        ui->lblConnection->setText(tr("<font color=\"darkRed\">Connected to Modbus TCP port</font>"));
        ui->spinPort->setEnabled(false);
        ui->btnConnect->setVisible(false);
        ui->btnDisconnect->setVisible(true);
        ui->boxRequest->setEnabled(true);
        ui->boxCycle->setEnabled(true);
        ui->boxAction->setEnabled(false);
        emit tabText(QString("Modbus Client [%1]").arg(ui->spinPort->value()));
    } else {
        ui->lblConnection->setText(tr("<font color=\"black\">Choose TCP port to connect</font>"));
        ui->spinPort->setEnabled(true);
        ui->btnConnect->setVisible(true);
        ui->btnDisconnect->setVisible(false);
        ui->boxRequest->setEnabled(false);
        ui->boxCycle->setEnabled(false);
        ui->boxAction->setEnabled(true);
        emit tabText(QString("Modbus Client [-]"));
    }
}

/********************************************************/
