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
        FileAppend
    };

    explicit FileHandler(QObject *parent = nullptr);

    ~FileHandler();

    QByteArray processData(const QByteArray& data);
    QByteArray processData(const QString& data);
    void doConnect(bool binary = false);
    void doDisconnect();

private:
    QFile m_File;
};

#endif // FILEHANDLER_H
