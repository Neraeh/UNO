#include "card.h"

Card::Card(QString _color, QString _id)
{
    color = _color.toUpper(), id = _id.toUpper();
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
        else if (color == "Y")
            card += ",08";
        else if (color == "G")
            card += ",03";
        else
            card += ",00""\x03""00,01";
        card += "[" + id + "]\x0F";
        return card;
    }
    else
        return "[" + (id == "+4" || id == "J" ? "" : color + ",") + id + "]";
}
