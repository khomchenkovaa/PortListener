#include "dbhandlerwidget.h"
#include "ui_dbhandlerwidget.h"

#include <QMessageBox>
#include <QFileDialog>

#include <QSqlDatabase>
#include <QSqlError>

/********************************************************/

DbHandlerWidget::DbHandlerWidget(QWidget *parent) :
    MessageHandlerWgt(parent),
    ui(new Ui::DbHandlerWidget)
{
    ui->setupUi(this);

    for (QString drv : QSqlDatabase::drivers()) {
        QString desc = getDescription(drv);
        if (desc.isNull()) {
            ui->comboType->addItem(drv, drv);
        } else {
            ui->comboType->addItem(QString("%1 (%2)").arg(desc, drv), drv);
        }
    }
    ui->comboType->setCurrentIndex(0);
}

/********************************************************/

DbHandlerWidget::~DbHandlerWidget()
{
    delete ui;
}

/********************************************************/

SettingsMap DbHandlerWidget::settings() const
{
    const QString driver   = ui->comboType->itemData( ui->comboType->currentIndex() ).toString();
    const QString hostname = ui->editHostname->text();
    const int     port     = ui->spinPort->value();
    const QString username = ui->editUsername->text();
    const QString password = ui->editPassword->text();
    const QString database = ui->editDatabase->text();

    SettingsMap map;
    map.insert(DbHandler::DbDriver, driver);
    map.insert(DbHandler::DbHostname, hostname);
    map.insert(DbHandler::DbPort, port);
    map.insert(DbHandler::DbUsername, username);
    map.insert(DbHandler::DbPassword, password);
    map.insert(DbHandler::DbDatabase, database);
    return map;
}

/********************************************************/

void DbHandlerWidget::setSettings(const SettingsMap &map)
{
    const QString driver   = map.value(DbHandler::DbDriver, "QPSQL").toString();
    const QString hostname = map.value(DbHandler::DbHostname, "localhost").toString();
    const int     port     = map.value(DbHandler::DbPort, 0).toInt();
    const QString username = map.value(DbHandler::DbUsername).toString();
    const QString password = map.value(DbHandler::DbPassword).toString();
    const QString database = map.value(DbHandler::DbDatabase).toString();

    ui->comboType->setCurrentIndex( ui->comboType->findData(driver) );
    ui->editHostname->setText(hostname);
    ui->spinPort->setValue(port);
    ui->editUsername->setText(username);
    ui->editPassword->setText(password);
    ui->editDatabase->setText(database);
}

/********************************************************/

void DbHandlerWidget::on_comboType_currentIndexChanged(int index)
{
    QString driver = ui->comboType->itemData(index).toString();
    updateFields(driver);
}

/********************************************************/

void DbHandlerWidget::on_buttonSelectFile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose a SQLite database file"),
                                                        QString(),
                                                        "SQLite databases (*.db);;All Files (*.*)");
    if (filename.isEmpty()) return;
    ui->editDatabase->setText(filename);
}

/********************************************************/

void DbHandlerWidget::on_btnTest_clicked()
{
    const QString driver   = ui->comboType->itemData( ui->comboType->currentIndex() ).toString();
    const QString hostname = ui->editHostname->text();
    const int     port     = ui->spinPort->value();
    const QString username = ui->editUsername->text();
    const QString password = ui->editPassword->text();
    const QString database = ui->editDatabase->text();

    if (!QSqlDatabase::isDriverAvailable(driver)) {
        QMessageBox::critical(this, tr("Testing Connection"),
                              tr("Connection failed:\nDatabase driver %1 is not available.")
                              .arg(driver));
        return;
    }

    QUuid dbuuid = QUuid::createUuid();
    QSqlDatabase db = QSqlDatabase::addDatabase(driver, dbuuid.toString());
    if (!db.isValid()) {
        QMessageBox::critical(this, tr("Testing Connection"),
                              tr("Connection failed:\n%1")
                              .arg("Database is not valid"));
        return;
    }

    if (driver == "QSQLITE" || driver == "QSQLITE2") {
        db.setDatabaseName(database);
    } else {
        db.setHostName(hostname);
        if (port > 0) db.setPort(port);
        db.setDatabaseName(database);
        db.setUserName(username);
        db.setPassword(password);
    }

    if (db.open()) {
        QMessageBox::information(this, tr("Testing Connection"),
                                 tr("Connection established successfully."));
        db.close();
    } else {
        QMessageBox::critical(this, tr("Testing Connection"),
                              tr("Connection failed:\n%1")
                              .arg(db.lastError().text()));
    }
    QSqlDatabase::removeDatabase(dbuuid.toString());
}

/********************************************************/

void DbHandlerWidget::updateFields(const QString &drv)
{
    if (drv == "QSQLITE" || drv == "QSQLITE2") {
        ui->editHostname->setEnabled(false);
        ui->spinPort->setEnabled(false);
        ui->editUsername->setEnabled(false);
        ui->editPassword->setEnabled(false);
        ui->labelDatabase->setText("Filename");
        ui->buttonSelectFile->setVisible(true);
    } else {
        ui->editHostname->setEnabled(true);
        ui->spinPort->setEnabled(true);
        ui->editUsername->setEnabled(true);
        ui->editPassword->setEnabled(true);
        ui->labelDatabase->setText("Database");
        ui->buttonSelectFile->setVisible(false);
    }
}

/********************************************************/

QString DbHandlerWidget::getDescription(const QString& drv)
{
    if (0) return "";
    if (drv == "QDB2")	   return "IBM DB2";
    if (drv == "QIBASE")   return "Borland InterBase";
    if (drv == "QOCI")	   return "Oracle Call Interface";
    if (drv == "QODBC")	   return "ODBC";
    if (drv == "QODBC3")   return "ODBC";
    if (drv == "QTDS")	   return "Sybase Adaptive Server";
    if (drv == "QMYSQL")   return "MySQL 4.x";
    if (drv == "QMYSQL3")  return "MySQL 3.x";
    if (drv == "QPSQL")	   return "PostgreSQL 8.x";
    if (drv == "QPSQL7")   return "PostgreSQL 7.x";
    if (drv == "QSQLITE")  return "SQLite 3.x";
    if (drv == "QSQLITE2") return "SQLite 2.x";
    return QString();
}

/********************************************************/
