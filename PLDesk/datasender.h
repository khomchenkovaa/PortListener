#ifndef DATASENDER_H
#define DATASENDER_H

#include "listenerwidget.h"

namespace Ui {
class DataSender;
}

class DataSender : public ListenerWidget
{
    Q_OBJECT

public:
    explicit DataSender(QWidget *parent = nullptr);
    ~DataSender();

protected:
    QTextBrowser *textLog() const;

private Q_SLOTS:
    void doSend();
    void doConnect();
    void onInputFormatChanged();
    void changeHandler(int index);

private:
    /// configure UI default state
    void setupUiDefaultState();
    void updateCodecs();

private:
    Ui::DataSender *ui;
};

#endif // DATASENDER_H
