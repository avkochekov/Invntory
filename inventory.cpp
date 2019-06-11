#include <inventory.h>
#include <QTableWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QSizePolicy>
#include <QScrollBar>
#include <QMap>

Inventory::Inventory(QWidget *parent) : QTableWidget(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    horizontalHeader()->setVisible(false);
    horizontalHeader()->setDefaultSectionSize(80);
    horizontalHeader()->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(80);
    verticalHeader()->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    verticalHeader()->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}

int Inventory::createInventory(int rows, int cols)
{
    setRowCount(rows);
    setColumnCount(cols);
    return createInventory();
}

QByteArray Inventory::getInventory()
{
    QJsonArray cells;
    foreach (int key, items.keys()){
        cells.append(QJsonObject{
                         {"position", key},
                         {"type", items.value(key).first},
                         {"count", items.value(key).second}
                     });
    }
    QJsonObject inventary;
    inventary.insert("rows", rowCount());
    inventary.insert("cols", colorCount());
    inventary.insert("cells", cells);
    return QJsonDocument(inventary).toJson();
}

void Inventory::setInventory(QByteArray array)
{
    clearContents();
    QJsonObject inventory = QJsonDocument::fromJson(array).object();
    int rows = inventory.value("rows").toInt();
    int cols = inventory.value("cols").toInt();
    if (items.count() > rows * cols){
        qDebug() << "Bad inventory size! Abort!";
        return;
    }
    cellCount = rows * cols;
    setRowCount(rows);
    setColumnCount(cols);
    QJsonArray items = inventory.value("cells").toArray();
    foreach (QJsonValue itemValue, items) {
        QJsonObject item = itemValue.toObject();

        int position = item.value("position").toInt();
        ItemType type = ItemType(item.value("type").toInt());
        int count = item.value("count").toInt();

        createItem(position, type, count);
    }
    emit created();

}

int Inventory::createInventory()
{
    clearContents();
    cellCount = rowCount() * columnCount();
    for (int position = 0; position < cellCount; position++){
        createItem(position);
    }
    emit created();
    return cellCount;
}

void Inventory::createItem(int position, ItemType type, int count)
{
    Item *apple = new Item(position);

    connect(apple, SIGNAL(increased(int, ItemType)), this, SLOT(increaseItem(int, ItemType)));
    connect(apple, SIGNAL(decreased(int)), this, SLOT(decreaseItem(int)));
    connect(apple, SIGNAL(appended(int, int)), this, SLOT(mergeItems(int, int)));

    connect(this, &Inventory::itemDeleted, apple, &Item::playAudio);
    connect(this, &Inventory::itemUpdated, apple, &Item::updateItem);

    if (position < 0)
        updateItem(position, type, 1);
    else
        updateItem(position, type, count);

    int row = position/rowCount();
    int col = position%columnCount();
    setCellWidget(row, col, apple);
    emit itemCreated(position, type);
}

void Inventory::increaseItem(int position, ItemType type)
{
    if (items.value(position).first != type && items.value(position).first != None)
        return;
    updateItem(position, type, items.value(position).second+1);
}

void Inventory::decreaseItem(int position)
{
    QPair<ItemType,int> item = items.value(position);
    if (item.second == 0 || item.first == None)
        return;
    int count = item.second - 1;
    ItemType type;
    if (count){
        type = item.first;
    } else {
        type = None;
        emit itemDeleted();
    }
    updateItem(position, type, count);
}

void Inventory::mergeItems(int start, int stop)
{
    QPair<ItemType,int> startItem = items.value(start);
    QPair<ItemType,int> stopItem  = items.value(stop);

    if (startItem.first == stopItem.first){
        int count = stopItem.second+startItem.second;
        ItemType type = startItem.first;

        updateItem(start, None, 0);
        updateItem(stop, type, count);
    } else {
        updateItem(start, startItem.first, startItem.second);
        updateItem(stop, stopItem.first, stopItem.second);
    }
}

void Inventory::updateItem(int position, ItemType type, int count)
{
    QPair<ItemType,int> item = QPair<ItemType,int>(type, count);
    if (items.value(position).first == type && items.value(position).second == count && items.contains(position))
        return;
    items.insert(position, item);
    itemUpdated(position, type, count);
}

void Inventory::updateItem(QByteArray array)
{
    QJsonObject item = QJsonDocument().fromJson(array).object();
    int position = item.value("position").toInt();
    int type = item.value("type").toInt();
    int count = item.value("count").toInt();
    updateItem(position, ItemType(type), count);
}

QByteArray Inventory::getItem(int position, ItemType type, int count)
{
    QJsonObject item;
    item.insert("position", position);
    item.insert("type", type);
    item.insert("count", count);
    return QJsonDocument(item).toJson();
}
