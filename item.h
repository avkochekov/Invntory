#ifndef ITEM_H
#define ITEM_H

#include <QLabel>
#include <itemtype.h>
#include <QMediaPlayer>

class Item : public QWidget
{
    Q_OBJECT
//public:
//    enum ItemType{None, Apple};

private:
    QString m_image = QString();
    QString m_audio = QString();
    QPoint m_dragStart;

    QLabel *imageLabel;
    QLabel *countLabel;
    int position = -1;
    bool infinity = false;
    ItemType type;
    QMediaPlayer *player = new QMediaPlayer;

public:
    explicit Item(int position = -1, ItemType type = None);

    void setBorder();
    void setCount(int count = 0);
    void setInfinity(bool i = false);
    void setType(ItemType type = ItemType::None);
    void setInventaryPosition(int position);

    int getPosition(){return position;}
    ItemType getType(){return type;}

private slots:
    void setImage();
    void setAudio();

protected:
    void mousePressEvent( QMouseEvent* event );
    void mouseMoveEvent( QMouseEvent* event );
    void dragEnterEvent( QDragEnterEvent* event );
    void dropEvent( QDropEvent* event );

signals:
    void created(int position, ItemType type);
    void increased(int position, ItemType type);
    void appended(int start, int stop);
    void decreased(int position);
    void disappeared();
    void typeChanged();

public slots:
    void updateItem(int index, ItemType type, int count);
    void playAudio();
};

#endif // ITEM_H
