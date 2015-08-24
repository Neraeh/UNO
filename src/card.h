#ifndef CARD_H
#define CARD_H

#include <QString>

class Card
{
public:
    explicit Card(QString _color, QString _id);
    QString getColor() const;
    QString getId() const;
    QString toString(bool colored = true) const;
    inline bool operator==(const Card& other) const;

private:
    QString color, id;
};

#endif // CARD_H
