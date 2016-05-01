#ifndef CARDS_H
#define CARDS_H

#include "uno.h"

class UNO;
class Card;
class Cards
{
public:
    Cards(UNO *_parent);

    Card* get(int i) const
    {
        return cards.at(i);
    }

    int size() const
    {
        return cards.size();
    }

    Card* first() const
    {
        return cards.first();
    }

    Card* last() const
    {
        return cards.last();
    }

    void randomize();
    Card *pick(Card* _card = 0);

    bool isEmpty() const
    {
        return cards.isEmpty();
    }

private:
    UNO *parent;
    QList<Card*> cards, picked;
};

#endif // CARDS_H
