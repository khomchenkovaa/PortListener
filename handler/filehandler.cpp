#include "filehandler.h"

#include "filehandlerwidget.h"

#include <QTextStream>

/********************************************************/

FileHandler::FileHandler(QObject *parent)
    : MessageHandler(parent)
{
    m_Name = tr("File handler");
}

/********************************************************/

FileHandler::~FileHandler()
{
    doDisconnect();
}

/********************************************************/

void FileHandler::handleMessage(Message *msg)
{
    m_Error.clear();
    if (!m_Connected || !m_File.isOpen()) {
        m_Error = tr("Cannot write data to file");
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
    m_Error.clear();
    if (m_Connected && m_File.isOpen()) {
        m_File.write(data);
    } else {
        m_Error = tr("Cannot write binary data to file");
    }
    return QByteArray();
}

/********************************************************/

QByteArray FileHandler::processData(const QString &data)
{
    m_Error.clear();
    if (m_Connected && m_File.isOpen()) {
        QTextStream out(&m_File);
        out << data;
    } else {
        m_Error = tr("Cannot write text data to file");
    }
    return QByteArray();
}

/********************************************************/

void FileHandler::doConnect(bool binary)
{
    m_Error.clear();
    QString fileName = m_Settings.value(Settings::FileName).toString();
    if (fileName.isEmpty()) {
        m_Error = tr("No file to open");
    }
    m_File.setFileName(fileName);
    QIODevice::OpenMode flags;
    if (m_Settings.value(Settings::FileAppend, true).toBool()) {
        flags = QIODevice::Append;
    } else {
        flags = QIODevice::WriteOnly;
    }
    if (!binary) {
        flags |= QIODevice::Text;
    }
    if (!m_File.open(flags)) {
        m_Error = tr("Could not open file");
    }
    m_Connected = true;
}

/********************************************************/

void FileHandler::doDisconnect()
{
    if (m_File.isOpen()) {
        m_File.close();
    }
    m_Connected = false;
}

/********************************************************/

MessageHandlerWgt *FileHandler::settingsWidget(QWidget *parent) const
{
    return new FileHandlerWidget(parent);
}

/********************************************************/
