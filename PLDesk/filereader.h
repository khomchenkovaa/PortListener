#ifndef FILEREADER_H
#define FILEREADER_H

#include "listenerwidget.h"

#include <QTimer>
#include <QFile>
#include <QFileInfo>

namespace Ui {
class FileReader;
}

class FileReader : public ListenerWidget
{
    Q_OBJECT

    enum ReadMode {
        READ_ALL,   ///< read all file content
        READ_CHUNK  ///< read line by line (text files) or number of bytes (bin files)
    };

    enum TextFileFormat {
        PLAIN_TEXT,  ///< text file
        HEX_CONTENT, ///< convert hex-text to binary output
        STRUCTURE_1  ///< special data format
    };

    struct FileReaderData {
        QFile          file;
        bool           binary = false;
        bool           rotate = false;
        QTimer         timer;
        ReadMode       readMode = ReadMode::READ_ALL;
        std::size_t    binLength = 12;                              ///< chunk size for binary input
        TextFileFormat textFileFormat = TextFileFormat::PLAIN_TEXT; ///< text file format

        QString fileName() const {
            return QFileInfo(file).fileName();
        }

        bool isBin() const {
            return binary || (textFileFormat == TextFileFormat::HEX_CONTENT);
        }
    };

public:
    explicit FileReader(QWidget *parent = nullptr);
    ~FileReader();

signals:
    void tabText(const QString &label);

protected:
    QTextBrowser *textLog() const;

private Q_SLOTS:
    void onReadyRead();
    void doConnect();
    void doDisconnect();
    void onInputFormatChanged();
    void changeDecoder(int index);
    void changeReplyType(int index);
    void changeHandler(int index);

private:
    /// configure UI default state
    void setupUiDefaultState();
    void updateStatus();
    void updateCodecs();
    QByteArray processData(const QString &fileName, const QByteArray &data);

private:
    Ui::FileReader *ui;
    FileReaderData d;
};

#endif // FILEREADER_H
