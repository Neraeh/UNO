#ifndef CARDS_H
#define CARDS_H

#include <QList>
#include "card.h"

class Cards
{
public:
    explicit Cards();
    Card* get(int i) const;
    int size() const;
    void remove(int i);
    bool isEmpty() const;

private:
    QList<Card*> cards;
};

#endif // CARDS_H
