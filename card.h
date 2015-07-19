#ifndef CARD_H
#define CARD_H

#include <QObject>

class Card : public QObject
{
    Q_OBJECT
public:
    explicit Card(QString _color, QString _id);
    QString getColor();
    QString getId();
    bool equals(Card _arg);
    QString toString();

private:
    QString color, id;
};

#endif // CARD_H
