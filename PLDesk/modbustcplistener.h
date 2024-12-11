#ifndef MODBUSTCPLISTENER_H
#define MODBUSTCPLISTENER_H

#include "listenerwidget.h"
#include "xmodbustcpserver.h"

#include <QBitArray>

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

public Q_SLOTS:
    void onDataUpdated(QModbusDataUnit::RegisterType table, int address, int size);
    void handleDeviceError(QModbusDevice::Error newError);

protected:
    QTextBrowser *textLog() const;

private Q_SLOTS:
    void doConnect();
    void doDisconnect();
    void activateHandler();
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
    XModbusTcpServer m_ModbusDevice;
};

#endif // MODBUSTCPLISTENER_H
