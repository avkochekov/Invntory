#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <inventory.h>

#include <QPushButton>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    /// Создает основное окно программы с размерами 360x320
    /// В качестве основного компановщика используется QStackedLayout
    /// В компановшик добавляются виджеты
    ///     Основного меню
    ///     Меню выбора подключения
    ///     Виджет игрового поля

    ui->setupUi(this);
    setFixedSize(360,320);
    setWindowTitle("Инвентарь");

    stacked = new QStackedLayout();
    stacked->addWidget(getMainMenu());
    stacked->addWidget(getPlayField());

    centralWidget()->setLayout(stacked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QWidget *MainWindow::getMainMenu()
{
    /// Возвращает виджет с кнопками "Новая игра" и "Выход"
    /// При клике по кнопке "Новая игра" отображается Окно сообщения
    /// с выбором роли - клиент или сервер.
    /// При клике по кнопке "Выход" компановщик stacked
    /// отображает следующий виджет.

    QWidget *w = new QWidget();
    QVBoxLayout *vBox = new QVBoxLayout;
    QPushButton *pbNewGame = new QPushButton(this);
    QPushButton *pbExit = new QPushButton(this);

    connect(pbNewGame, &QPushButton::clicked, [=](){
        setWindowTitle("Инвентарь");
        QMessageBox msgBox(QMessageBox::NoIcon,
                           "Новая игра",
                           "Для начала игры и создания пустого инвентаря выберите \"Сервер\".\n"
                           "Для подключения к уже существующему инвентарю выберите \"Клиент\".",
                           QMessageBox::NoButton);
        msgBox.addButton("Клиент", QMessageBox::AcceptRole);
        msgBox.addButton("Сервер", QMessageBox::RejectRole);
        if (msgBox.exec() == QMessageBox::AcceptRole){
            connect(network, &Network::newMessage, inventory, &Inventory::setInventory);
            connect(network, &Network::connected, [=](){
                stacked->setCurrentIndex(1);
            });
            connect(network, &Network::noHostAvailable, [=](){
                QMessageBox msgBox(QMessageBox::Warning,
                                   "Новая игра",
                                   "Отсутствует игра для подключения.\n"
                                   "Выберите другой режим игры или попробуйтe подключиться еще раз.",
                                   QMessageBox::Ok);
                msgBox.exec();
            });
            connect(inventory, &Inventory::created, [=](){
                disconnect(network, &Network::newMessage, inventory, &Inventory::setInventory);
                connect(network, SIGNAL(newMessage(QByteArray)), inventory, SLOT(updateItem(QByteArray)));
            });
            network->serachServer();
        } else {
            setWindowTitle("Инвентарь - Сервер");
            inventory->createInventory(3,3);
            connect(network, &Network::connected, [=](){
                network->sendMessage(inventory->getInventory());
                connect(network, SIGNAL(newMessage(QByteArray)), inventory, SLOT(updateItem(QByteArray)));
            });
            network->createServer();
            stacked->setCurrentIndex(1);
        }
    });
    connect(pbExit, &QPushButton::clicked, [=](){QApplication::exit();});


    pbNewGame->setMinimumHeight(50);
    pbExit->setMinimumHeight(50);

    pbNewGame->setText("Новая игра");
    pbExit->setText("Выход");

    vBox->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    vBox->addWidget(pbNewGame);
    vBox->addWidget(pbExit);
    vBox->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));


    w->setLayout(vBox);
    return w;
}

QWidget *MainWindow::getPlayField()
{
    /// Возвращает виджет с игровым полем, на котором расположены инвентарь,
    /// источкин объектов и кнопка, возвращающая игрока в главное меню
    QWidget *w = new QWidget();

    QPushButton *pb = new QPushButton("Главое меню",this);
    pb->setMinimumHeight(50);
    connect(pb, &QPushButton::clicked, [=](){
        stacked->setCurrentIndex(0);
        setWindowTitle("Инвентарь");
        network->disconnect();
    });

///    Раскомментировать, есть необходима очистка инвентаря при "новой игре"
    connect(pb, SIGNAL(clicked()), inventory, SLOT(createInventory()));

    inventory = new Inventory(this);

    connect(inventory, &Inventory::itemUpdated, [=](int position, ItemType type, int count){
        network->sendMessage(inventory->getItem(position, type, count));
    });

    connect(inventory, &Inventory::itemUpdated, db, &DataBase::updateItem);
    connect(inventory, &Inventory::itemCreated, db, &DataBase::insertItem);

    Item *apple = new Item();
    apple->setType(Apple);
    apple->setInfinity(true);
    apple->setBorder();
    apple->setAcceptDrops(false);

    QGridLayout *gBox = new QGridLayout();
    gBox->addWidget(pb,0,0,1,2);
    gBox->addWidget(inventory,1,0, Qt::AlignCenter);
    gBox->addWidget(apple,1,1, Qt::AlignCenter);

    w->setLayout(gBox);
    return w;
}
