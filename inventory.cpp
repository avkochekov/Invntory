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
    /// Конструктор. Создает инвентарь - таблицу без заголовков
    /// с фиксированной высотой и шириной ячеек

    setSelectionMode(QAbstractItemView::NoSelection);
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
    /// Создает таблицу - инвентарь и наполняет ее ячейками для хранения предметов
    /// Устанавливает количество строк и колонок таблицы.
    /// Рассчитывает количество позиций для ячеек инвентаря
    /// и для каждой позиции создает ячейку-виджет.
    /// Возврашает количество созданных ячеек инвентаря

    clearContents();
    setRowCount(rows);
    setColumnCount(cols);
    cellCount = rowCount() * columnCount();
    for (int position = 0; position < cellCount; position++){
        createItem(position);
    }
    emit created();
    return cellCount;
}

QByteArray Inventory::getInventory()
{
    /// Возвращает информацию о размерности инвентаря
    /// и параметрах каждой его ячейки
    /// (позиция, тип хранимого предмета, количество предметов в пачке).
    /// Данные упаковываются в строку формата JSon для дальнейшей передачи по сети.

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
    inventary.insert("cols", columnCount());
    inventary.insert("cells", cells);
    return QJsonDocument(inventary).toJson();
}

void Inventory::setInventory(QByteArray array)
{

    /// Принимает в качестве аргумента данные в формате JSon
    /// Распаковывает данные: размерность инвентаря, параметры каждой ячейки
    /// (позиция, тип хранимого предмета, количество предметов в пачке);
    /// создает по полученным данным новый инвентарь и заполняет его предметами

    clearContents();
    setRowCount(0);
    setColumnCount(0);
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

void Inventory::createItem(int position, ItemType type, int count)
{
    /// Создает ячейку инвентаря.
    /// Подключаются сигналы и слоты:
    ///     увеличение количества предметов ячейки на 1
    ///     уменьшение количества предметов ячейки на 1
    ///     перемещение предметов из одной ячейки в другую
    /// а так же изменения изображения ячейки и проигрывания аудио
    /// в случае удаления последнего предмета

    Item *apple = new Item(position,type,count);

    connect(apple, SIGNAL(increased(int, ItemType)), this, SLOT(increaseItem(int, ItemType)));
    connect(apple, SIGNAL(decreased(int)), this, SLOT(decreaseItem(int)));
    connect(apple, SIGNAL(merged(int, int)), this, SLOT(mergeItems(int, int)));

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
    /// Увеличивает количество предметов в ячейке на 1
    /// Присутствует проверка соответствия типа добавляемого предмета
    /// и предмета, хранящегося в ячейке инвентаря.

    if (items.value(position).first != type && items.value(position).first != None)
        return;
    updateItem(position, type, items.value(position).second+1);
}

void Inventory::decreaseItem(int position)
{
    /// Уменьшает количество предметов в ячейке на 1
    /// Присутствует проверка на пустую ячейку - в этом случае
    /// уменьшение количества предметов не происходит

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
    /// Объединяет предметы при перемещении их из одной ячейки в другую не пустую ячейку.
    /// Объединение происходит, если типы предметов совпадают, в противном случае
    /// пачки прдметов меняются ячейками.

    QPair<ItemType,int> startItem = items.value(start);
    QPair<ItemType,int> stopItem  = items.value(stop);

    if (startItem.first == stopItem.first){
        int count = stopItem.second+startItem.second;
        ItemType type = startItem.first;

        updateItem(start, None, 0);
        updateItem(stop, type, count);
    } else {
        updateItem(stop, startItem);
        updateItem(start, stopItem);
    }
}

void Inventory::updateItem(int position, QPair<ItemType, int> item)
{
    /// Обновляет значения ячейки инвентаря в позиции position

    items.insert(position, item);
    itemUpdated(position, item.first, item.second);
}

void Inventory::updateItem(int position, ItemType type, int count)
{
    /// Обновляет значения ячейки инвентаря в позиции position

    QPair<ItemType,int> item = QPair<ItemType,int>(type, count);
    if (items.value(position).first == type && items.value(position).second == count && items.contains(position))
        return;
    updateItem(position, item);
}

void Inventory::updateItem(QByteArray array)
{
    /// Принимает в качестве аргумента данные в формате JSon
    /// Распаковывает данные: позицию ячейки, тип предмета и количество предметов в пачке;
    /// обновляет по полученным данным значение ячейки инвентаря

    QJsonObject item = QJsonDocument().fromJson(array).object();
    int position = item.value("position").toInt();
    int type = item.value("type").toInt();
    int count = item.value("count").toInt();
    updateItem(position, ItemType(type), count);
}

QByteArray Inventory::getItem(int position, ItemType type, int count)
{
    /// Возвращает информацию о ячейке инвентаря:
    ///     позиция
    ///     тип хранимого предмета
    ///     количество предметов в пачке
    /// Данные упаковываются в строку формата JSon для дальнейшей передачи по сети.

    QJsonObject item;
    item.insert("position", position);
    item.insert("type", type);
    item.insert("count", count);
    return QJsonDocument(item).toJson();
}
