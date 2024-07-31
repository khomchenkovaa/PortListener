#ifndef GATEHANDLERCONF_H
#define GATEHANDLERCONF_H

#include "xcsvmodel.h"
#include "dbhandlerconf.h"

#include <QFile>
#include <QTextCodec>

class GateCsvConf
{
    enum {
        NumColumn,
        TypeColumn,
        NameColumn,
        DimColumn,
        KksColumn,
        PointColumn
    };

    struct GateCsvConfItem {
        quint32 num;   ///< номер
        QString type;  ///< тип
        QString name;  ///< Наименование сигнала
        QString dim;   ///< Размерность
        QString kks;   ///< Код KKS
        QString point; ///< пункт
    };

public:
    void load(const QString &fileName) {
        XCsvModel csv;
        csv.setSource(fileName, true, ',', QTextCodec::codecForName("Windows-1251"));
        for (int i = 0; i < csv.rowCount(); ++i) {
            GateCsvConfItem item;
            item.num   = csv.data(csv.index(i, NumColumn)).toUInt();
            item.type  = csv.data(csv.index(i, TypeColumn)).toString();
            item.name  = csv.data(csv.index(i, NameColumn)).toString();
            item.dim   = csv.data(csv.index(i, DimColumn)).toString();
            item.kks   = csv.data(csv.index(i, KksColumn)).toString();
            item.point = csv.data(csv.index(i, PointColumn)).toString();
            items << item;
        }
    }

private:
    QList<GateCsvConfItem> items;
};

class GateDefConf {

    enum {
        NameColumn,
        TypeColumn,
        OffsetColumn,
        LayersColumn,
        RowsColumn,
        ColumnsColumn
    };

    struct GateDefConfItem {
        QString name;        ///< Name
        QString type;        ///< тип
        quint64 offset  = 0; ///< Offset
        quint16 layers  = 1; ///< Layers
        quint16 rows    = 1; ///< Rows
        quint16 columns = 1; ///< Columns
    };

    struct GateDefConfItemPrivate {
        quint32 fields = 0;  ///< Count of fields
        quint64 size   = 4;  ///< Size in bytes
        QList<GateDefConfItem> items;
    };

public:
    void load(const QString &fileName) {
        quint8 state = 0;
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        while (!file.atEnd()) {
            QString line = file.readLine().trimmed();
            if (line.isEmpty()) continue;
            if (line.startsWith('#')) continue;
            if (state == 0) {
                d.fields = line.toUInt();
                state = 1;
                continue;
            }
            if (state == 1) {
                d.size = line.toUInt();
                state = 2;
                continue;
            }
            const auto values = CsvDbMapper::parseCsv(line, ',');
            GateDefConfItem item;
            if (values.size() > NameColumn)    item.name    = values.at(NameColumn);
            if (values.size() > TypeColumn)    item.type    = values.at(TypeColumn);
            if (values.size() > OffsetColumn)  item.offset  = values.at(OffsetColumn).toUInt();
            if (values.size() > LayersColumn)  item.layers  = values.at(LayersColumn).toUInt();
            if (values.size() > RowsColumn)    item.rows    = values.at(RowsColumn).toUInt();
            if (values.size() > ColumnsColumn) item.columns = values.at(ColumnsColumn).toUInt();
            d.items << item;
        }

        file.close();
    }

private:
    GateDefConfItemPrivate d;
};

#endif // GATEHANDLERCONF_H
