#ifndef MODBUSTCPCLIENT_H
#define MODBUSTCPCLIENT_H

#include "listenerwidget.h"

#include <QModbusTcpClient>

namespace Ui {
class ModbusTcpClient;
}

class ModbusTcpClient : public ListenerWidget
{
    Q_OBJECT

public:
    explicit ModbusTcpClient(QWidget *parent = nullptr);
    ~ModbusTcpClient();

signals:
    void tabText(const QString &label);

public slots:
    void doModbusRequest();
    void handleDeviceError(QModbusDevice::Error newError);

private slots:
    void doConnect();
    void doDisconnect();
    void changeHandler(int index);
    void printInfo(const QString& host, const QString& msg);
    void printMessage(const QString& host, const QString& msg);
    void printError(const QString& host, const QString& msg);

private:
    /// configure UI default state
    void setupUiDefaultState();
    void updateStatus();

private:
    Ui::ModbusTcpClient *ui;
    QModbusTcpClient m_ModbusDevice;
};

#endif // MODBUSTCPCLIENT_H
