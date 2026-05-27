#ifndef DTSCONNECT_H
#define DTSCONNECT_H

#include "listenerwidget.h"

#include "dtsclient.h"

#include <QTimer>
#include <QMutex>

namespace Ui {
class DtsConnect;
}

class DtsConnect : public ListenerWidget
{
    Q_OBJECT

    struct DtsValue {
        int32_t   type;     ///< Тип значения (enum ValueType)
        int32_t   idx;      ///< Индекс значения
        int32_t    quality; ///< Битовое качество
        QDateTime dt;       ///< Время создания
        QVariant  value;    ///< Значение

        DtsValue(Value & v);
    };

    struct DtsConnectData {
        QScopedPointer<DTSClient> dtsClient;
        QTimer timer;
        QMutex mutex;
    };

public:
    explicit DtsConnect(QWidget *parent = nullptr);
    ~DtsConnect();

signals:
    void dataReceived(DtsConnect::DtsValue v, int32_t chnlId);
    void tabText(const QString &label);

protected:
    QTextBrowser *textLog() const;

private Q_SLOTS:
    void doConnect();
    void doDisconnect();
    void doReceive(DtsConnect::DtsValue v, int32_t chnlId);

private:
    /// configure UI default state
    void setupUiDefaultState();
    void updateStatus();

private: // static
    /// Функция регистрации принятых данных
    static int rcvFunc( void * argPtr, Value & value, int32_t chnlId );

private:
    Ui::DtsConnect *ui;
    DtsConnectData d;
};

#endif // DTSCONNECT_H
