#ifndef INVENTORY_H
#define INVENTORY_H

#include <QWidget>
#include <QTableWidget>
#include <QMap>
#include <QPair>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <itemtype.h>
#include <item.h>

class Inventory : public QTableWidget
{
    Q_OBJECT
public:
    explicit Inventory(QWidget *parent = nullptr);
    int createInventory(int rows, int cols);

    QByteArray getInventory();

public slots:
    int createInventory();
    void setInventory(QByteArray inventory);



private:
    QMap<int,QPair<ItemType,int>> items;
    int cellCount;

signals:
    void mainMenu();
    void itemUpdated(int position, ItemType type, int count);
    void itemCreated(int position, ItemType type);
    void itemDeleted();
    void created();


private slots:
    void createItem(int position = -1, ItemType type = None, int count = 0);
    void increaseItem(int position, ItemType type);
    void decreaseItem(int position);
    void mergeItems(int start, int stop);
    void updateItem(int position, ItemType type, int count);

public slots:
    void updateItem(QByteArray);
    QByteArray getItem(int position, ItemType type, int count);
//    void itemUpdated(QByteArray array);


};

#endif // INVENTORY_H
