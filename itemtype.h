#ifndef ITEMTYPE_H
#define ITEMTYPE_H

#include <QStringList>

enum ItemType{None, Apple};

const QStringList itemsName = {
    QString(),
    "Яблоко"
};

const QStringList imagesPath = {
    QString(),
    ":/images/apple"
};

const QStringList audiosPath = {
    QString(),
    "qrc:/audios/apple"
};

#endif // ITEMTYPE_H
