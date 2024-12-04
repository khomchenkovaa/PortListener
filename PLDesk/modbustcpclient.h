#ifndef MODBUSTCPCLIENT_H
#define MODBUSTCPCLIENT_H

#include "listenerwidget.h"
#include "modbusclientconf.h"

#include <QModbusTcpClient>

namespace Ui {
class ModbusTcpClient;
}

class ModbusTcpClient : public ListenerWidget
{
    Q_OBJECT

    struct ModbusTcpClientData {
        QModbusTcpClient modbusDevice;
        ModbusSigConf    conf;
        QTimer           *timer = Q_NULLPTR;
    };

public:
    explicit ModbusTcpClient(QWidget *parent = nullptr);
    ~ModbusTcpClient();

signals:
    void tabText(const QString &label);

public slots:
    void doReadRequest();
    void doWriteRequest();
    void doModbusRequest();
    void handleDeviceError(QModbusDevice::Error newError);

private slots:
    void doConnect();
    void doDisconnect();
    void changeHandler(int index);
    void printInfo(const QString& host, const QString& msg);
    void printMessage(const QString& host, const QString& msg);
    void printError(const QString& host, const QString& msg);
    void onReadReady();

private:
    /// configure UI default state
    void setupUiDefaultState();
    void setupSingleRequestBlock();
    void setupCycleRequestBlock();
    void updateStatus();

private:
    Ui::ModbusTcpClient *ui;
    ModbusTcpClientData  m;
};

#endif // MODBUSTCPCLIENT_H
