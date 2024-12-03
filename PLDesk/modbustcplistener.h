#ifndef MODBUSTCPLISTENER_H
#define MODBUSTCPLISTENER_H

#include "listenerwidget.h"

#include <QModbusTcpServer>
#include <QBitArray>

#define REG_MAX 2000

namespace Ui {
class ModbusTcpListener;
}

class ModbusTcpListener : public ListenerWidget
{
    Q_OBJECT

public:
    explicit ModbusTcpListener(QWidget *parent = nullptr);
    ~ModbusTcpListener();

signals:
    void tabText(const QString &label);

public slots:
    void onDataUpdated(QModbusDataUnit::RegisterType table, int address, int size);
    void handleDeviceError(QModbusDevice::Error newError);

private slots:
    void doConnect();
    void doDisconnect();
    void activateHandler();
    void printInfo(const QString& host, const QString& msg);
    void printMessage(const QString& host, const QString& msg);
    void printError(const QString& host, const QString& msg);
    void updateEditorStatus();
    void doReadValue();
    void doWriteValue();

private:
    /// configure UI default state
    void setupUiDefaultState();
    void setupEditor();
    void updateStatus();
    void onCoilsUpdated(int address, int size);
    void onHoldingRegistersUpdated(int address, int size);
    void processCoils(int address, int size, const QBitArray &data);
    void processHoldingRegisters(int address, int size, const QVariantList &data);

private:
    Ui::ModbusTcpListener *ui;
    QModbusTcpServer m_ModbusDevice;
};

#endif // MODBUSTCPLISTENER_H
