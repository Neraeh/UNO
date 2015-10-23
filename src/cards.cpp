#include "cards.h"

Cards::Cards(UNO *_parent)
{
    parent = _parent;

    cards.append(new Card("R", "0"));
    cards.append(new Card("R", "1"));
    cards.append(new Card("R", "1"));
    cards.append(new Card("R", "2"));
    cards.append(new Card("R", "2"));
    cards.append(new Card("R", "3"));
    cards.append(new Card("R", "3"));
    cards.append(new Card("R", "4"));
    cards.append(new Card("R", "4"));
    cards.append(new Card("R", "5"));
    cards.append(new Card("R", "5"));
    cards.append(new Card("R", "6"));
    cards.append(new Card("R", "6"));
    cards.append(new Card("R", "7"));
    cards.append(new Card("R", "7"));
    cards.append(new Card("R", "8"));
    cards.append(new Card("R", "8"));
    cards.append(new Card("R", "9"));
    cards.append(new Card("R", "9"));
    cards.append(new Card("B", "0"));
    cards.append(new Card("B", "1"));
    cards.append(new Card("B", "1"));
    cards.append(new Card("B", "2"));
    cards.append(new Card("B", "2"));
    cards.append(new Card("B", "3"));
    cards.append(new Card("B", "3"));
    cards.append(new Card("B", "4"));
    cards.append(new Card("B", "4"));
    cards.append(new Card("B", "5"));
    cards.append(new Card("B", "5"));
    cards.append(new Card("B", "6"));
    cards.append(new Card("B", "6"));
    cards.append(new Card("B", "7"));
    cards.append(new Card("B", "7"));
    cards.append(new Card("B", "8"));
    cards.append(new Card("B", "8"));
    cards.append(new Card("B", "9"));
    cards.append(new Card("B", "9"));
    cards.append(new Card("Y", "0"));
    cards.append(new Card("Y", "1"));
    cards.append(new Card("Y", "1"));
    cards.append(new Card("Y", "2"));
    cards.append(new Card("Y", "2"));
    cards.append(new Card("Y", "3"));
    cards.append(new Card("Y", "3"));
    cards.append(new Card("Y", "4"));
    cards.append(new Card("Y", "4"));
    cards.append(new Card("Y", "5"));
    cards.append(new Card("Y", "5"));
    cards.append(new Card("Y", "6"));
    cards.append(new Card("Y", "6"));
    cards.append(new Card("Y", "7"));
    cards.append(new Card("Y", "7"));
    cards.append(new Card("Y", "8"));
    cards.append(new Card("Y", "8"));
    cards.append(new Card("Y", "9"));
    cards.append(new Card("Y", "9"));
    cards.append(new Card("G", "0"));
    cards.append(new Card("G", "1"));
    cards.append(new Card("G", "1"));
    cards.append(new Card("G", "2"));
    cards.append(new Card("G", "2"));
    cards.append(new Card("G", "3"));
    cards.append(new Card("G", "3"));
    cards.append(new Card("G", "4"));
    cards.append(new Card("G", "4"));
    cards.append(new Card("G", "5"));
    cards.append(new Card("G", "5"));
    cards.append(new Card("G", "6"));
    cards.append(new Card("G", "6"));
    cards.append(new Card("G", "7"));
    cards.append(new Card("G", "7"));
    cards.append(new Card("G", "8"));
    cards.append(new Card("G", "8"));
    cards.append(new Card("G", "9"));
    cards.append(new Card("G", "9"));
    cards.append(new Card("R", "+2"));
    cards.append(new Card("R", "+2"));
    cards.append(new Card("B", "+2"));
    cards.append(new Card("B", "+2"));
    cards.append(new Card("Y", "+2"));
    cards.append(new Card("Y", "+2"));
    cards.append(new Card("G", "+2"));
    cards.append(new Card("G", "+2"));
    cards.append(new Card("R", "I"));
    cards.append(new Card("R", "I"));
    cards.append(new Card("B", "I"));
    cards.append(new Card("B", "I"));
    cards.append(new Card("Y", "I"));
    cards.append(new Card("Y", "I"));
    cards.append(new Card("G", "I"));
    cards.append(new Card("G", "I"));
    cards.append(new Card("R", "P"));
    cards.append(new Card("R", "P"));
    cards.append(new Card("B", "P"));
    cards.append(new Card("B", "P"));
    cards.append(new Card("Y", "P"));
    cards.append(new Card("Y", "P"));
    cards.append(new Card("G", "P"));
    cards.append(new Card("G", "P"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "J"));
    cards.append(new Card("N", "J"));
    cards.append(new Card("N", "J"));
    cards.append(new Card("N", "J"));
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
