#ifndef DECK_H
#define DECK_H

#include <QObject>
#include <QList>
#include <QTime>
#include "echos.h"
#include "card.h"

class Deck : public QObject
{
    Q_OBJECT
public:
    explicit Deck(Echos *_parent);
    QString randCards(int _count);
    void remCard(QString _color, QString _id);
    bool contains(Card *_card) const;
    bool containsColor(QString _color) const;
    bool containsId(QString _id) const;
    int size() const;
    bool isEmpty() const;
    QString toString() const;

private:
    Echos *parent;
    QList<Card*> cards;
};

#endif // DECK_H
