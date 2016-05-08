#include "card.h"

Card::Card(Color _color, QString _id)
{
    color = _color, id = _id.toUpper();
}

Card::Card(QString _color, QString _id)
{
    color = toColor(_color), id = _id.toUpper();
}

QString Card::toString(bool colored) const
{
    if (colored)
    {
        QString card = "\x02""\x03""01,";

        switch(color)
        {
        case RED:
            card += "04";
            break;
        case GREEN:
            card += "03";
            break;
        case BLUE:
            card += "11";
            break;
        case YELLOW:
            card += "08";
            break;
        default:
            card += "00""\x03""00,01";
        }

        card += "[" + id + "]\x0F";
        return card;
    }
    else
        return "[" + QString(id == "+4" || id == "J" ? "" : toString(color) + ",") + id + "]";
}

QString Card::toString(Color color)
{
    switch (color)
    {
    case RED:
        return QObject::tr("R", "Red letter ingame (translate to first letter of the color in your language)");
    case GREEN:
        return QObject::tr("G", "Green letter ingame (translate to first letter of the color in your language)");
    case BLUE:
        return QObject::tr("B", "Blue letter ingame (translate to first letter of the color in your language)");
    case YELLOW:
        return QObject::tr("Y", "Yellow letter ingame (translate to first letter of the color in your language)");
    default:
        return QString();
    }
}

Color Card::toColor(QString color)
{
    color = color.toUpper();
    if (color == QObject::tr("R", "Red letter ingame (translate to first letter of the color in your language)"))
        return RED;
    else if (color == QObject::tr("G", "Green letter ingame (translate to first letter of the color in your language)"))
        return GREEN;
    else if (color == QObject::tr("B", "Blue letter ingame (translate to first letter of the color in your language)"))
        return BLUE;
    else if (color == QObject::tr("Y", "Yellow letter ingame (translate to first letter of the color in your language)"))
        return YELLOW;
    else
        return NONE;
}
