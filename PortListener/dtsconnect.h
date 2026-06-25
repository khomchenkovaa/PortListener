#ifndef DTSCONNECT_H
#define DTSCONNECT_H

#include "listenerwidget.h"

#include "dtsclient.h"

#include <QTimer>
#include <QMutex>

namespace Ui {
class DtsConnect;
}

struct DtsValue {
    int32_t   type;     ///< Тип значения (enum ValueType)
    int32_t   idx;      ///< Индекс значения
    int32_t   quality;  ///< Битовое качество
    QDateTime dt;       ///< Время создания
    QVariant  value;    ///< Значение

    DtsValue() {}
    DtsValue(Value & v);
};

Q_DECLARE_METATYPE(DtsValue)

class DtsAgent : public QObject
{
    Q_OBJECT

    DTSClient *dtsClient;

public:
    explicit DtsAgent(QObject *parent = Q_NULLPTR);
    ~DtsAgent();

    void receiveData(Value & value, int32_t chnlId);
    bool doConnect(quint16 port, QString *err = Q_NULLPTR);
    void doDisconnect();

    int put(Value & value, int32_t chnlId, QString *err = Q_NULLPTR);

    bool isActive() const {
        return dtsClient;
    }

signals:
    void dataReceived(DtsValue v, int32_t chnlId);

private: // static
    /// Функция регистрации принятых данных
    static int rcvFunc( void * argPtr, Value & value, int32_t chnlId );
};

class DtsConnect : public ListenerWidget
{
    Q_OBJECT

    struct DtsConnectData {
        QScopedPointer<DtsAgent> dtsAgent;
        QTimer timer;
        QMutex mutex;
    };

public:
    explicit DtsConnect(QWidget *parent = nullptr);
    ~DtsConnect();

signals:
    void tabText(const QString &label);

protected:
    QTextBrowser *textLog() const;

private Q_SLOTS:
    void doConnect();
    void doDisconnect();
    void doReceive(DtsValue v, int32_t chnlId);
    void doSendAValue();
    void doSendBValue();
    void doSendIValue();

private:
    /// configure UI default state
    void setupUiDefaultState();
    void updateStatus();

private:
    Ui::DtsConnect *ui;
    DtsConnectData d;
};

#endif // DTSCONNECT_H
