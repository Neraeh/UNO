#ifndef CARD_H
#define CARD_H

#include <QString>

class Card
{
public:
    Card(QString _color, QString _id);

    inline QString getColor() const
    {
        return color;
    }

    inline QString getId() const
    {
        return id;
    }

    QString toString(bool colored = true) const;

private:
    QString color, id;
};

#endif // CARD_H
