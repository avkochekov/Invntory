#include "database.h"
#include <QDebug>
#include <QDir>
#include <QFile>

#include <itemtype.h>

DataBase::DataBase()
{
    QSqlDatabase sdb = QSqlDatabase::addDatabase("QSQLITE");
    sdb.setDatabaseName(QDir::current().path() + "/database/database.sqlite");
    if (!sdb.open()) {
        qDebug() << sdb.lastError().text();
        return;
    }
    createTables();
}

void DataBase::createTables()
{
    QSqlQuery query;
    query.exec("CREATE TABLE Inventory "
                          "(`CellPosition` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                          "`ItemCount` INTEGER NOT NULL DEFAULT 0, "
                          "`ItemType` INTEGER DEFAULT 0)");
    query.exec("CREATE TABLE Item "
                              "(`Id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE, "
                              "`Image` TEXT, "
                              "`Name` TEXT)");

    int count = 0;
    foreach (QString path, imagesPath){
        query.prepare("INSERT INTO Item (Id, Image, Name) "
                            "VALUES (:id, :image, :name)");
        query.bindValue(":id", count);
        query.bindValue(":image", path);
        query.bindValue(":name", itemsName.at(count));
        query.exec();
        count++;
    }
}

void DataBase::insertItem(int id, int type)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Inventory (CellPosition, ItemType) "
                        "VALUES (:id, :type)");
    query.bindValue(":id", id);
    query.bindValue(":type", type);
    query.exec();
}

void DataBase::updateItem(int id, int type, int count)
{
    QSqlQuery query;
    query.prepare("UPDATE Inventory "
                  "SET ItemType = :type, ItemCount = :count "
                  "WHERE CellPosition = :id");
    query.bindValue(":id", id);
    query.bindValue(":type", type);
    query.bindValue(":count", count);
    query.exec();
}

