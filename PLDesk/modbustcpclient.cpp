#include "modbustcpclient.h"
#include "ui_modbustcpclient.h"

#include "messagehandlerwgt.h"

#include <QMessageBox>
#include <QTimer>
#include <QFileInfo>
#include <QFileDialog>


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

    connect(ui->btnConfig, &QAbstractButton::clicked,
            this, &ModbusTcpClient::openSigFileDialog);
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

void ModbusTcpClient::doModbusRequest()
{
    if (m_ModbusDevice.state() == QModbusDevice::ConnectedState) {
        QString host = QString("%1:%2").arg(ui->editHost->text()).arg(ui->spinPort->value());
        printInfo(host, "Start request cycle");

        auto time = QTime::currentTime().msecsSinceStartOfDay() / 1000;
        printMessage(host, QString::number(time));

        const auto table =
                static_cast<QModbusDataUnit::RegisterType>(ui->cmbTable->currentData().toInt());
        int serverId = ui->spinServerId->value();

        for (const auto &item : qAsConst(conf.items)) {
            QModbusDataUnit request(table, item.ad, item.addressInterval());
            if (auto *response = m_ModbusDevice.sendReadRequest(request, serverId)) {
                if (!response->isFinished())
                    connect(response, &QModbusReply::finished, this, [this, item](){
                        auto reply = qobject_cast<QModbusReply *>(sender());
                        if (!reply) return;

                        if (reply->error() == QModbusDevice::NoError) {
                            const QModbusDataUnit unit = reply->result();
                            switch(item.dt) {
                            case Modbus::RealType: {
                                Modbus::ModbusValue v;
                                v.in.first = unit.value(0);
                                v.in.last  = unit.value(1);
                                auto host = QString("%1 [%2 %3]")
                                        .arg(item.pin)
                                        .arg(v.in.first, 4, 16, QLatin1Char('0'))
                                        .arg(v.in.last, 4, 16, QLatin1Char('0'));
                                auto info = QString::number(v.outFloat, 'f', 3);
//                                displayData << QString("%1;%2").arg(item.pin, info);
                                printMessage(host, info);
                            } break;
                            case Modbus::DWordType: {
                                Modbus::ModbusValue v;
                                v.in.first = unit.value(0);
                                v.in.last  = unit.value(1);
                                auto host = QString("%1 [%2 %3]")
                                        .arg(item.pin)
                                        .arg(v.in.first, 4, 16, QLatin1Char('0'))
                                        .arg(v.in.last, 4, 16, QLatin1Char('0'));
                                auto info = QString::number(v.outInt);
//                                displayData << QString("%1;%2").arg(item.pin, info);
                                printMessage(host, info);
                            } break;
                            case Modbus::IntType: {
                                quint16 val = unit.value(0);
                                auto host = QString("%1 [%2]")
                                        .arg(item.pin)
                                        .arg(val, 4, 16, QLatin1Char('0'));
                                auto info = QString::number(val);
//                                displayData << QString("%1;%2").arg(item.pin, info);
                                printMessage(host, info);
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
                printError(item.pin, m_ModbusDevice.errorString());
            }
        }
    }

    int msec = ui->spinFrequency->value() * 1000;
    QTimer::singleShot(msec, this, &ModbusTcpClient::doModbusRequest);
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

    loadSigConfig();

    if (m_ModbusDevice.state() != QModbusDevice::UnconnectedState) {
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

    doModbusRequest();
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

void ModbusTcpClient::openSigFileDialog()
{
    QString dirPath = QCoreApplication::applicationDirPath();
    QString fileName = ui->editConfig->text();
    const QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        dirPath = fileInfo.path();
    }

    fileName = QFileDialog::getOpenFileName(this, tr("Choose Sig Config File"),
                                            dirPath,
                                            "SIG files (*.sig)");
    if (!fileName.isEmpty()) {
        ui->editConfig->setText(fileName);
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

void ModbusTcpClient::loadSigConfig()
{
    QString sigFile = ui->editConfig->text().trimmed();
    if (sigFile.isEmpty()) sigFile = ":/resources/scrload/scrload.sig";
    conf.items.clear();
    conf.load(sigFile);
}

/********************************************************/
