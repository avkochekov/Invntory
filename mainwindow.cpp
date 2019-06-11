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
    ui->setupUi(this);
    setFixedSize(360,320);

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
    QWidget *w = new QWidget();
    QVBoxLayout *vBox = new QVBoxLayout;
    QPushButton *pbNewGame = new QPushButton(this);
    QPushButton *pbExit = new QPushButton(this);

    connect(pbNewGame, &QPushButton::clicked, [=](){stacked->setCurrentIndex(1);});
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
    QWidget *w = new QWidget();
    QVBoxLayout *vBox = new QVBoxLayout;
    QPushButton *pbClient = new QPushButton(this);
    QPushButton *pbServer = new QPushButton(this);
    network = new Network();

    connect(pbClient, &QPushButton::clicked, [=](){
        connect(network, &Network::newMessage, inventory, &Inventory::setInventory);
        connect(inventory, &Inventory::created, [=](){
            disconnect(network, &Network::newMessage, inventory, &Inventory::setInventory);
            connect(network, SIGNAL(newMessage(QByteArray)), inventory, SLOT(updateItem(QByteArray)));
            connect(inventory, &Inventory::itemUpdated, [=](int position, ItemType type, int count){
                network->sendMessage(inventory->getItem(position, type, count));
            });
        });
        network->serachServer();
        stacked->setCurrentIndex(2);
    });

    connect(pbServer, &QPushButton::clicked, [=](){
        inventory->createInventory(3,3);
        connect(network, &Network::connected, [=](){
            network->sendMessage(inventory->getInventory());
        });
//        connect(inventory, &Inventory::itemUpdated, network, &Network::)
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
    QWidget *w = new QWidget();

    QPushButton *pb = new QPushButton("Главое меню",this);
    pb->setMinimumHeight(50);
    connect(pb, &QPushButton::clicked, [=](){
        stacked->setCurrentIndex(0);
        disconnect(network, SLOT(newMessage));
        disconnect(network, SLOT(connected));
    });

///    Раскомментировать, есть необходима очистка инвентаря при "новой игре"
//    connect(pb, SIGNAL(clicked()), inventory, SLOT(createInventory()));

    inventory = new Inventory(this);

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
