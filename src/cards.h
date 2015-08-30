#ifndef CARDS_H
#define CARDS_H

#include "uno.h"

class UNO;
class Card;
class Cards
{
public:
    explicit Cards(UNO *_parent);
    Card* get(int i) const;
    int size() const;
    Card* first() const;
    Card* last() const;
    void randomize();
    Card *pick(Card* _card = 0);
    bool isEmpty() const;

private:
    UNO *parent;
    QList<Card*> cards, picked;
};

#endif // CARDS_H
