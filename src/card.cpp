#include "card.h"

Card::Card(QString _color, QString _id) : QObject()
{
    color = _color.toUpper(), id = _id.toUpper();
}

QString Card::getColor() const
{
    return color;
}

QString Card::getId() const
{
    return id;
}

inline bool Card::operator==(const Card& other) const
{
    if (getId() == other.getId() && getColor() == other.getColor())
        return true;
    else
        return false;
}

QString Card::toString(bool colored) const
{
    if (colored)
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
            card += ",00""\x03""00,01";
        card += "[" + id + "]\x0F";
        return card;
    }
    else
        return "[" + (id == "+4" || id == "J" ? "" : color + ",") + id + "]";
}
