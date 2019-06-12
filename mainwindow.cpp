#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <inventory.h>

#include <QPushButton>
#include <QTcpSocket>
#include <QTcpServer>

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
    stacked->addWidget(getConnectionMenu());
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
    /// При клике по кнопке "Новая игра" компановщик stacked
    /// отображает следующий виджет.
    /// При клике по кнопке "Выход" компановщик stacked
    /// отображает следующий виджет.

    QWidget *w = new QWidget();
    QVBoxLayout *vBox = new QVBoxLayout;
    QPushButton *pbNewGame = new QPushButton(this);
    QPushButton *pbExit = new QPushButton(this);

    connect(pbNewGame, &QPushButton::clicked, [=](){
        setWindowTitle("Инвентарь - Новая игра");
        stacked->setCurrentIndex(1);
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

QWidget *MainWindow::getConnectionMenu()
{
    /// Возвращает виджет с кнопками "Клиент" и "Сервер"
    /// Здесть игрок выбирает, будет ли он сервером, либо клиентом
    /// Если игрок выбирает клиента и подключение к серверу не произошло,
    /// В приложении отображается виджет "Главное меню"
    /// Если игрок выбирает клиента и успешно подключается к серверу,
    /// или выбирает сервер, отображается виджет "Игровое поле"
    QWidget *w = new QWidget();
    QVBoxLayout *vBox = new QVBoxLayout;
    QPushButton *pbClient = new QPushButton(this);
    QPushButton *pbServer = new QPushButton(this);
    network = new Network();

    connect(pbClient, &QPushButton::clicked, [=](){
        setWindowTitle("Инвентарь - Клиент");
        connect(network, &Network::newMessage, inventory, &Inventory::setInventory);
        connect(network, &Network::connected, [=](){
            stacked->setCurrentIndex(2);
        });
        connect(network, &Network::noHostAvailable, [=](){
            stacked->setCurrentIndex(0);
        });
        connect(inventory, &Inventory::created, [=](){
            disconnect(network, &Network::newMessage, inventory, &Inventory::setInventory);
            connect(network, SIGNAL(newMessage(QByteArray)), inventory, SLOT(updateItem(QByteArray)));
        });
        network->serachServer();
    });

    connect(pbServer, &QPushButton::clicked, [=](){
        setWindowTitle("Инвентарь - Сервер");
        inventory->createInventory(3,3);
        connect(network, &Network::connected, [=](){
            network->sendMessage(inventory->getInventory());
            connect(network, SIGNAL(newMessage(QByteArray)), inventory, SLOT(updateItem(QByteArray)));
        });
        network->createServer();
        stacked->setCurrentIndex(2);
    });

    pbClient->setText("Клиент");
    pbServer->setText("Сервер");

    pbClient->setMinimumHeight(50);
    pbServer->setMinimumHeight(50);


    vBox->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding));
    vBox->addWidget(pbClient);
    vBox->addWidget(pbServer);
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
//        disconnect(network, SLOT(newMessage));
//        disconnect(network, SLOT(connected));
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
