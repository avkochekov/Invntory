#include "item.h"
#include <QPixmap>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDir>
#include <QMediaPlayer>

#include <QDebug>

Item::Item(int position, ItemType type)
{
    setAcceptDrops(true);
    setMinimumSize(80,80);

    countLabel = new QLabel(this);
    imageLabel = new QLabel(this);
    countLabel->setMinimumSize(this->minimumSize());
    countLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    countLabel->setMargin(3);
    countLabel->setTextFormat(Qt::TextFormat::RichText);
    imageLabel->setMinimumSize(this->minimumSize());

    setInfinity(false);
    setInventaryPosition(position);

    if (type){
        setType(type);
    }
    emit created(position, type);

    connect(this, &Item::typeChanged, &Item::setImage);
    connect(this, &Item::typeChanged, &Item::setAudio);
}

void Item::setImage()
{
    m_image = imagesPath.at(type);
    if (m_image.isNull())
        imageLabel->clear();
    else
        imageLabel->setPixmap(QPixmap(m_image).scaled(imageLabel->minimumSize()));
}

void Item::setInfinity(bool i)
{
    infinity = i;
    if (infinity)
        countLabel->setText("∞");

}

void Item::setType(ItemType type)
{
    this->type = type;
    emit typeChanged();
}

void Item::setInventaryPosition(int position)
{
    this->position = position;
}

void Item::setAudio()
{
    m_audio = audiosPath.at(type);
}

void Item::playAudio()
{
    player->setMedia(QUrl(m_audio));
    player->setVolume(100);
    player->play();
    connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
        [=](QMediaPlayer::Error error){ qDebug() << error << ":     " << player->errorString();});
}

void Item::setCount(int count)
{
    if (!count){
        countLabel->clear();
        emit disappeared();
    } else {
        countLabel->setText(QString::number(count));
    }
}

void Item::setBorder()
{
    setStyleSheet("QLabel {"
                 "border-style: solid;"
                 "border-width: 1px;"
                 "border-color: black; "
                 "}");
}

void Item::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton){
        m_dragStart = event->pos();
    }
    else if (event->buttons() & Qt::RightButton){
        emit decreased(position);
    }

}

void Item::mouseMoveEvent(QMouseEvent *event)
{
    if((event->buttons() & Qt::LeftButton ) &&
            !m_image.isNull() &&
            QApplication::startDragDistance() <= ( event->pos() - m_dragStart ).manhattanLength()
    ){
        QDrag* drag = new QDrag( this );
        QMimeData* mimeData = new QMimeData;
        mimeData->setImageData(imageLabel->pixmap());
        mimeData->setData("type",QString::number(type).toUtf8());
        mimeData->setData("position",QString::number(position).toUtf8());
        drag->setMimeData( mimeData );
        drag->setPixmap(*imageLabel->pixmap());
        drag->setHotSpot(event->pos()-imageLabel->pos());


        Qt::DropAction result = drag->exec( Qt::MoveAction );
        if(result == Qt::MoveAction && !infinity){
            setType(None);
            setCount();
        }
    }
}

void Item::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasImage()) {
        event->acceptProposedAction();
    }
}

void Item::dropEvent(QDropEvent *event)
{
    ItemType type = ItemType(event->mimeData()->data("type").toInt());
    int startPosition = event->mimeData()->data("position").toInt();
    int stopPosition = this->position;

    if (startPosition == stopPosition)
        return;

    if (startPosition < 0)
        emit increased(stopPosition, type);
    else
        emit appended(startPosition,stopPosition);
}

void Item::updateItem(int index, ItemType type, int count)
{
    if (index != position)
        return;
    setCount(count);
    setType(type);
}
