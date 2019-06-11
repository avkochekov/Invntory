#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QObject>
#include <QSqlError>
#include <QSqlQuery>


class DataBase : public QObject
{
    Q_OBJECT
public:
    DataBase();

private:
    QSqlDatabase db;
    void createTables();

public slots:
    void insertItem(int id, int type);
    void updateItem(int id, int type, int count);
};

#endif // DATABASE_H
