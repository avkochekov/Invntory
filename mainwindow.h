#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedLayout>

#include <database.h>
#include <network.h>
#include <inventory.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QWidget *getMainMenu();
    QWidget *getConnectionMenu();
    QWidget *getPlayField();

private:
    Ui::MainWindow *ui;
    QStackedLayout *stacked;
    DataBase *db = new DataBase();
    Network *network = new Network();
    Inventory *inventory = new Inventory();

signals:
    void exit();
    void newGame();
    void inventoryChanged();
};

#endif // MAINWINDOW_H
