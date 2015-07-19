#include "card.h"

Card::Card(QString _color, QString _id) : QObject()
{
    color = _color.toUpper();
    id = _id.toUpper();
}

QString Card::getColor()
{
    return color;
}

QString Card::getId()
{
    return id;
}

bool Card::equals(Card _arg)
{
    if (_arg.getId() == id && _arg.getColor() == color)
        return true;
    else
        return false;
}

QString Card::toString()
{
    QString card = "\x02""\x03""01";

    if (color == "R")
        card += ",04";
    else if (color == "B")
        card += ",11";
    else if (color == "J")
        card += ",08";
    else if (color == "V")
        card += ",03";
    else
        card += "\x03""00,01";

    card += "[" + id + "]\x0F";
    return card;
}
