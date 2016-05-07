#include "cards.h"

Cards::Cards(UNO *_parent)
{
    parent = _parent;

    cards.append(new Card(RED, "0"));
    cards.append(new Card(RED, "1"));
    cards.append(new Card(RED, "1"));
    cards.append(new Card(RED, "2"));
    cards.append(new Card(RED, "2"));
    cards.append(new Card(RED, "3"));
    cards.append(new Card(RED, "3"));
    cards.append(new Card(RED, "4"));
    cards.append(new Card(RED, "4"));
    cards.append(new Card(RED, "5"));
    cards.append(new Card(RED, "5"));
    cards.append(new Card(RED, "6"));
    cards.append(new Card(RED, "6"));
    cards.append(new Card(RED, "7"));
    cards.append(new Card(RED, "7"));
    cards.append(new Card(RED, "8"));
    cards.append(new Card(RED, "8"));
    cards.append(new Card(RED, "9"));
    cards.append(new Card(RED, "9"));
    cards.append(new Card(BLUE, "0"));
    cards.append(new Card(BLUE, "1"));
    cards.append(new Card(BLUE, "1"));
    cards.append(new Card(BLUE, "2"));
    cards.append(new Card(BLUE, "2"));
    cards.append(new Card(BLUE, "3"));
    cards.append(new Card(BLUE, "3"));
    cards.append(new Card(BLUE, "4"));
    cards.append(new Card(BLUE, "4"));
    cards.append(new Card(BLUE, "5"));
    cards.append(new Card(BLUE, "5"));
    cards.append(new Card(BLUE, "6"));
    cards.append(new Card(BLUE, "6"));
    cards.append(new Card(BLUE, "7"));
    cards.append(new Card(BLUE, "7"));
    cards.append(new Card(BLUE, "8"));
    cards.append(new Card(BLUE, "8"));
    cards.append(new Card(BLUE, "9"));
    cards.append(new Card(BLUE, "9"));
    cards.append(new Card(YELLOW, "0"));
    cards.append(new Card(YELLOW, "1"));
    cards.append(new Card(YELLOW, "1"));
    cards.append(new Card(YELLOW, "2"));
    cards.append(new Card(YELLOW, "2"));
    cards.append(new Card(YELLOW, "3"));
    cards.append(new Card(YELLOW, "3"));
    cards.append(new Card(YELLOW, "4"));
    cards.append(new Card(YELLOW, "4"));
    cards.append(new Card(YELLOW, "5"));
    cards.append(new Card(YELLOW, "5"));
    cards.append(new Card(YELLOW, "6"));
    cards.append(new Card(YELLOW, "6"));
    cards.append(new Card(YELLOW, "7"));
    cards.append(new Card(YELLOW, "7"));
    cards.append(new Card(YELLOW, "8"));
    cards.append(new Card(YELLOW, "8"));
    cards.append(new Card(YELLOW, "9"));
    cards.append(new Card(YELLOW, "9"));
    cards.append(new Card(GREEN, "0"));
    cards.append(new Card(GREEN, "1"));
    cards.append(new Card(GREEN, "1"));
    cards.append(new Card(GREEN, "2"));
    cards.append(new Card(GREEN, "2"));
    cards.append(new Card(GREEN, "3"));
    cards.append(new Card(GREEN, "3"));
    cards.append(new Card(GREEN, "4"));
    cards.append(new Card(GREEN, "4"));
    cards.append(new Card(GREEN, "5"));
    cards.append(new Card(GREEN, "5"));
    cards.append(new Card(GREEN, "6"));
    cards.append(new Card(GREEN, "6"));
    cards.append(new Card(GREEN, "7"));
    cards.append(new Card(GREEN, "7"));
    cards.append(new Card(GREEN, "8"));
    cards.append(new Card(GREEN, "8"));
    cards.append(new Card(GREEN, "9"));
    cards.append(new Card(GREEN, "9"));
    cards.append(new Card(RED, "+2"));
    cards.append(new Card(RED, "+2"));
    cards.append(new Card(BLUE, "+2"));
    cards.append(new Card(BLUE, "+2"));
    cards.append(new Card(YELLOW, "+2"));
    cards.append(new Card(YELLOW, "+2"));
    cards.append(new Card(GREEN, "+2"));
    cards.append(new Card(GREEN, "+2"));
    cards.append(new Card(RED, "R"));
    cards.append(new Card(RED, "R"));
    cards.append(new Card(BLUE, "R"));
    cards.append(new Card(BLUE, "R"));
    cards.append(new Card(YELLOW, "R"));
    cards.append(new Card(YELLOW, "R"));
    cards.append(new Card(GREEN, "R"));
    cards.append(new Card(GREEN, "R"));
    cards.append(new Card(RED, "S"));
    cards.append(new Card(RED, "S"));
    cards.append(new Card(BLUE, "S"));
    cards.append(new Card(BLUE, "S"));
    cards.append(new Card(YELLOW, "S"));
    cards.append(new Card(YELLOW, "S"));
    cards.append(new Card(GREEN, "S"));
    cards.append(new Card(GREEN, "S"));
    cards.append(new Card(NONE, "+4"));
    cards.append(new Card(NONE, "+4"));
    cards.append(new Card(NONE, "+4"));
    cards.append(new Card(NONE, "+4"));
    cards.append(new Card(NONE, "J"));
    cards.append(new Card(NONE, "J"));
    cards.append(new Card(NONE, "J"));
    cards.append(new Card(NONE, "J"));
}

void Cards::randomize()
{
    QList<Card*> temp;
    foreach (Card *w, cards)
    {
        temp.insert(temp.isEmpty() ? 0 : qrand() % temp.size(), w);
        cards.removeOne(w);
    }

    cards = temp;
}

Card* Cards::pick(Card *_card)
{
    Card *ret = _card == 0 ? cards.first() : _card;
    picked.append(ret);
    cards.removeOne(ret);

    if (cards.isEmpty())
    {
        while (!picked.isEmpty())
        {
            cards.append(picked.first());
            picked.removeOne(picked.first());
        }

        foreach (Player *w, parent->getPlayers()->getList())
            foreach (Card *c, w->getDeck()->getList())
                foreach (Card *i, cards)
                    if (i->getId() == c->getId() && i->getColor() == c->getColor())
                        cards.removeOne(i);
    }

    return ret;
}
