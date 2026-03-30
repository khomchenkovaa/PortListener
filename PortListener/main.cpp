#include "mainwindow.h"

#include <QApplication>

#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QLoggingCategory::setFilterRules("qt.modbus=true");
//    QLoggingCategory::setFilterRules("qt.modbus.lowlevel=true");

    MainWindow w;
    w.show();
    return a.exec();
}
