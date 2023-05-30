#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "messagehandler.h"

#include <QFile>

class FileHandler : public MessageHandler
{
    Q_OBJECT
public:
    explicit FileHandler(QObject *parent = nullptr);

    ~FileHandler();

    void processMessage(Message *msg);
    QByteArray processData(const QByteArray& data);
    QByteArray processData(const QString& data);
    void connect(bool binary = false);
    void disconnect();
    MessageHandlerWgt *settingsWidget(QWidget *parent = nullptr) const;

private:
    QFile m_File;
};

#endif // FILEHANDLER_H
