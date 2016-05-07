#ifndef CARD_H
#define CARD_H

#include <QObject>
#include <QString>

enum Color
{
    RED,
    GREEN,
    BLUE,
    YELLOW,
    NONE
};

class Card
{
public:
    Card(Color _color, QString _id);
    Card(QString _color, QString _id);

    Color getColor() const
    {
        return color;
    }

    QString getId() const
    {
        return id;
    }

    static QString toString(Color color);
    QString toString(bool colored = true) const;
    static Color toColor(QString color);

private:
    Color color;
    QString id;
};

#endif // CARD_H
