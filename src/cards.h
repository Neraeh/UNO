#ifndef CARDS_H
#define CARDS_H

#include "uno.h"

class UNO;
class Card;
class Cards
{
public:
    Cards(UNO *_parent);

    inline Card* get(int i) const
    {
        return cards.at(i);
    }

    inline int size() const
    {
        return cards.size();
    }

    inline Card* first() const
    {
        return cards.first();
    }

    inline Card* last() const
    {
        return cards.last();
    }

    void randomize();
    Card *pick(Card* _card = 0);

    inline bool isEmpty() const
    {
        return cards.isEmpty();
    }

private:
    UNO *parent;
    QList<Card*> cards, picked;
};

#endif // CARDS_H
