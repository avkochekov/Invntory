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
    /// Конструктор - создает ячейку с предметом, включающую 2 лейбла
    /// (для отображения изображения и количества предметов в пачке),
    /// способную обрабатывать drop-событие.
    /// Тип предмета, пути до изображения и аудио описаны в itemtype.h

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
    /// Метод устанавливает изображение ячейки в зависимости от типа предмета,
    /// расположенного в этой ячейки

    m_image = imagesPath.at(type);
    if (m_image.isNull())
        imageLabel->clear();
    else
        imageLabel->setPixmap(QPixmap(m_image).scaled(imageLabel->minimumSize()));
}

void Item::setInfinity(bool i)
{
    /// Метод указывает наличие бесконечного количества предметов в ячейке.
    /// Вызывается при создании ячеек - источников предметов.

    infinity = i;
    if (infinity)
        countLabel->setText("∞");

}

void Item::setType(ItemType type)
{
    /// Устанавливает тип предмета в ячейке.
    /// От типа зависит изображение ячейки и аудио

    this->type = type;
    emit typeChanged();
}

void Item::setInventaryPosition(int position)
{
    /// Указывает позицию ячейки в инвентаре
    /// Позиция рассчитывается по формуле [rowCount() * row + column]

    this->position = position;
}

void Item::setAudio()
{
    /// Устанавливает аудио для ячейки в зависимости от типа предмета,
    /// находящегося в ней

    m_audio = audiosPath.at(type);
}

void Item::playAudio()
{
    /// Проигрывает аудиофайл

    player->setMedia(QUrl(m_audio));
    player->setVolume(100);
    player->play();
    connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
        [=](QMediaPlayer::Error error){ qDebug() << error << ":     " << player->errorString();});
}

void Item::setCount(int count)
{
    /// Устанавливает количество предметов в пачке

    if (!count){
        countLabel->clear();
        emit disappeared();
    } else {
        countLabel->setText(QString::number(count));
    }
}

void Item::setBorder()
{
    /// Устанавливает рамку вокруг ячейки.
    /// Удобно для выделения источников предметов.

    setStyleSheet("QLabel {"
                 "border-style: solid;"
                 "border-width: 1px;"
                 "border-color: black; "
                 "}");
}

void Item::mousePressEvent(QMouseEvent *event)
{
    /// Обрабатывает нажатие кнопок мыши.
    /// В случае нажатия ЛКМ сохраняет позицию захвата
    /// В случае нажатия ПКМ сообщает о необходимости убрать один предмет
    /// из пачки в ячейке с позицией position

    if (event->buttons() & Qt::LeftButton){
        m_dragStart = event->pos();
    }
    else if (event->buttons() & Qt::RightButton){
        emit decreased(position);
    }

}

void Item::mouseMoveEvent(QMouseEvent *event)
{
    ///

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
    ///

    if(event->mimeData()->hasImage()) {
        event->acceptProposedAction();
    }
}

void Item::dropEvent(QDropEvent *event)
{
    ///

    ItemType type = ItemType(event->mimeData()->data("type").toInt());
    int startPosition = event->mimeData()->data("position").toInt();
    int stopPosition = this->position;

    if (startPosition == stopPosition)
        return;

    if (startPosition < 0)
        emit increased(stopPosition, type);
    else
        emit merged(startPosition,stopPosition);
}

void Item::updateItem(int index, ItemType type, int count)
{
    /// Обновляет ячейку инвентаря.
    /// Если позиция ячейки соответствует значению position,
    /// устанавливает количество предметов в пачке в значение count
    /// и тип предмета в значение type
    if (index != position)
        return;
    setCount(count);
    setType(type);
}
