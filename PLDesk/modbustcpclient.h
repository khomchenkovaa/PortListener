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

public Q_SLOTS:
    void doReadRequest();
    void doWriteRequest();
    void doModbusRequest();
    void handleDeviceError(QModbusDevice::Error newError);

protected:
    QTextBrowser *textLog() const;

private Q_SLOTS:
    void doConnect();
    void doDisconnect();
    void activateHandler();
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
