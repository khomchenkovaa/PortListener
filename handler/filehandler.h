#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "ihandler.h"

#include <QFile>

class FileHandler : public IHandler
{
    Q_OBJECT
public:
    explicit FileHandler(QObject *parent = nullptr);

    ~FileHandler();

    QByteArray processData(const QByteArray& data);
    QByteArray processData(const QString& data);
    void connect(bool binary = false);
    void disconnect();
    IHandlerWidget *settingsWidget(QWidget *parent = nullptr) const;

private:
    QFile m_File;
};

#endif // FILEHANDLER_H
