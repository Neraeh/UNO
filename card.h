#ifndef CARD_H
#define CARD_H

#include <QObject>

class Card : public QObject
{
    Q_OBJECT
public:
    explicit Card(QString _color, QString _id);
    QString getColor() const;
    QString getId() const;
    QString toString() const;
    inline bool operator==(const Card& other) const;

private:
    QString color, id;
};

#endif // CARD_H
