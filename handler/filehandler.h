#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "messagehandler.h"

#include <QFile>

class FileHandler : public MessageHandler
{
    Q_OBJECT

public:
    enum Settings {
        FileName,
        AppendToFile
    };

    explicit FileHandler(QObject *parent = nullptr);

    ~FileHandler();

    void handleMessage(Message *msg);
    QByteArray processData(const QByteArray& data);
    QByteArray processData(const QString& data);
    void connect(bool binary = false);
    void disconnect();
    MessageHandlerWgt *settingsWidget(QWidget *parent = nullptr) const;

private:
    QFile m_File;
};

#endif // FILEHANDLER_H
