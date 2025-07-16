#ifndef MQUEUELISTENER_H
#define MQUEUELISTENER_H

#include "listenerwidget.h"
#include "mqworker.h"

namespace Ui {
class MQueueListener;
}

class MQueueListener : public ListenerWidget
{
    Q_OBJECT

public:
    explicit MQueueListener(QWidget *parent = nullptr);
    ~MQueueListener();

signals:
    void tabText(const QString &label);

protected:
    QTextBrowser *textLog() const;

private Q_SLOTS:
    void onReadyRead();
    void doConnect();
    void doDisconnect();
    void onInputFormatChanged();
    void changeReplyType(int index);
    void changeHandler(int index);

private:
    /// configure UI default state
    void setupUiDefaultState();
    void updateStatus();
    void updateCodecs();
    QByteArray processData(const QString &mqName, const QByteArray &data);

private:
    Ui::MQueueListener *ui;
    MQWorker *mq_worker;
    QTimer   *mq_exchangeTimer; ///< таймер для записи/чтении в/из очередей, включается/выключается пользователем кнопками старт/стоп
};

#endif // MQUEUELISTENER_H
