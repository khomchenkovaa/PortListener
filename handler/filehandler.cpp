#include "filehandler.h"

#include <QTextStream>

/********************************************************/

FileHandler::FileHandler(QObject *parent)
    : MessageHandler(tr("File handler"), parent)
{
}

/********************************************************/

FileHandler::~FileHandler()
{
    FileHandler::doDisconnect();
}

/********************************************************/

void FileHandler::handleMessage(Message *msg)
{
    clearErrors();
    if (!isConnected() || !m_File.isOpen()) {
        addError(tr("Cannot write data to file"));
        return;
    }
    if (msg->payload.type() == QVariant::ByteArray) {
        m_File.write(msg->payload.toByteArray());
    } else {
        QTextStream out(&m_File);
        out << msg->payload.toString();
    }
}

/********************************************************/

QByteArray FileHandler::processData(const QByteArray &data)
{
    clearErrors();
    if (isConnected() && m_File.isOpen()) {
        m_File.write(data);
    } else {
        addError(tr("Cannot write binary data to file"));
    }
    return QByteArray();
}

/********************************************************/

QByteArray FileHandler::processData(const QString &data)
{
    clearErrors();
    if (isConnected() && m_File.isOpen()) {
        QTextStream out(&m_File);
        out << data;
    } else {
        addError(tr("Cannot write text data to file"));
    }
    return QByteArray();
}

/********************************************************/

void FileHandler::doConnect(bool binary)
{
    clearErrors();
    const auto fileName = settings()->value(Settings::FileName).toString();
    if (fileName.isEmpty()) {
        addError(tr("No file to open"));
    }
    m_File.setFileName(fileName);
    QIODevice::OpenMode flags;
    if (settings()->value(Settings::FileAppend, true).toBool()) {
        flags = QIODevice::Append;
    } else {
        flags = QIODevice::WriteOnly;
    }
    if (!binary) {
        flags |= QIODevice::Text;
    }
    if (!m_File.open(flags)) {
        addError(tr("Could not open file"));
    }
    setConnected();
}

/********************************************************/

void FileHandler::doDisconnect()
{
    if (m_File.isOpen()) {
        m_File.close();
    }
    setDisconnected();
}

/********************************************************/
