#ifndef DBHANDLERCONF_H
#define DBHANDLERCONF_H

#include <QString>
#include <QVariant>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include <QJsonDocument>
#include <QJsonArray>

/********************************************************/

struct DbFieldMapping {
    int index;
    QString name;
    QString type;
    QString format;
    QLocale locale;

    DbFieldMapping (const QVariantMap& map) :
        locale(QLocale::C) {
        index  = map.value("index", 0).toInt();
        name   = map.value("name").toString();
        type   = map.value("type").toString();
        format = map.value("format").toString();
    }

    QVariant value(const QString& str) const {
        if (str.isEmpty())      return QVariant();
        if (type == "String")   return str;
        if (type == "Int")      return locale.toInt(str);
        if (type == "Real")     return locale.toDouble(str);
        if (type == "DateTime") return QDateTime::fromString(str, format);
        return QVariant();
    }
};

/********************************************************/

struct DbTableMapping {
    QString prefix;
    QString table;
    QList<DbFieldMapping> fields;

    DbTableMapping (const QVariantMap& map) {
        prefix   = map.value("prefix").toString();
        table   = map.value("table").toString();
        for (auto item : map.value("fields").toList()) {
            const DbFieldMapping field(item.toMap());
            fields << field;
        }
    }

    QString sqlInsert() const {
        QStringList columns;
        QStringList values;
        for(auto item : fields) {
            columns << item.name;
            values  << QString(":%1").arg(item.name);
        }
        QString result = QString("INSERT INTO %1 (%2) VALUES (%3)")
                .arg(table, columns.join(", "), values.join(", "));
        return result;
    }

    QVariantMap bindValues(const QStringList& csv) const {
        QVariantMap result;
        for (int i=0; i<csv.size(); ++i) {
            int idx = findByIndex(i);
            if (idx != -1) {
                QString  key   = QString(":%1").arg(fields.at(idx).name);
                QVariant value = fields.at(idx).value(csv.at(i));
                if (value.isValid()) {
                    result.insert(key, value);
                }
            }
        }
        return result;
    }

    int findByIndex(int index) const {
        for (int i=0; i<fields.size(); ++i) {
            if (fields.at(i).index == index) {
                return i;
            }
        }
        return -1;
    }
};

/********************************************************/

class CsvDbMapper {
    QList<DbTableMapping> datamapper;
    QString error;

public:
    bool load(const QString& fileName = ":/resources/datamapper.json") {
        clear();
        QByteArray ba = readFile(fileName, &error);
        if (!error.isEmpty()) {
            return false;
        }
        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(ba, &jsonError);
        if (doc.isNull()) {
            error = jsonError.errorString();
            return false;
        }
        for (auto item : doc.array().toVariantList()) {
            const DbTableMapping table(item.toMap());
            datamapper << table;
        }
        return true;
    }

    void clear() {
        datamapper.clear();
        error.clear();
    }

    bool hasError() const {
        return !error.isEmpty();
    }

    QString loadError() const {
        return error;
    }

    QString sqlInsert(const QString& prefix) {
        int i = findByPrefix(prefix);
        if (i != -1) {
            return datamapper.at(i).sqlInsert();
        }
        return QString();
    }

    QVariantMap bindValues(const QString& prefix, QString& csv) {
        int i = findByPrefix(prefix);
        if (i != -1) {
            QStringList parsedCsv = parseCsv(csv);
            return datamapper.at(i).bindValues(parsedCsv);
        }
        return QVariantMap();
    }

private:
    int findByPrefix(const QString& prefix) const {
        for (int i=0; i<datamapper.size(); ++i) {
            if (prefix.compare(datamapper.at(i).prefix, Qt::CaseInsensitive) == 0) {
                return i;
            }
        }
        return -1;
    }

public:
    static QByteArray readFile(const QString& fileName, QString* err = Q_NULLPTR) {
        QFile file(fileName);
        QByteArray result;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            result = file.readAll();
            file.close();
        } else {
            if (err) err->append(file.errorString());
        }
        return result;
    }

    static QStringList parseCsv(QString& csv, QChar separator = ';') {
        QStringList row;
        QString field;
        QChar quote;
        QChar ch, buffer(0);
        QTextStream stream(&csv, QIODevice::ReadOnly);
        while(!stream.atEnd()) {
            if(buffer != QChar(0)) {
                ch = buffer;
                buffer = QChar(0);
            } else {
                stream >> ch;
            }
            if(ch != separator && (ch.category() == QChar::Separator_Line || ch.category() == QChar::Separator_Paragraph || ch.category() == QChar::Other_Control)) {
                row << field;
                field.clear();
                if(!row.isEmpty()) {
                    break;
                }
                row.clear();
            } else if(ch == '"') {
                quote = ch;
                do {
                    stream >> ch;
                    if(ch == quote) {
                        break;
                    }
                    field.append(ch);
                } while(!stream.atEnd());
            } else if(ch == separator) {
                row << field;
                field.clear();
            } else {
                field.append(ch);
            }
        }
        if(!field.isEmpty()) {
            row << field;
        }

        return row;
    }

    static QPair<QString, QString> splitToPrefixAndCsv(const QString& source, const QString& delimeter = ":") {
        QString prefix;
        QString csv;
        int idx = source.indexOf(delimeter);
        if (idx != -1) {
            prefix = source.left(idx);
            csv = source.mid(idx+1);
        }
        return QPair<QString, QString>(prefix,csv);
    }
};

/********************************************************/

#endif // DBHANDLERCONF_H
