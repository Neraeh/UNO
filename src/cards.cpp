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
    cards.append(new Card("J", "0"));
    cards.append(new Card("J", "1"));
    cards.append(new Card("J", "1"));
    cards.append(new Card("J", "2"));
    cards.append(new Card("J", "2"));
    cards.append(new Card("J", "3"));
    cards.append(new Card("J", "3"));
    cards.append(new Card("J", "4"));
    cards.append(new Card("J", "4"));
    cards.append(new Card("J", "5"));
    cards.append(new Card("J", "5"));
    cards.append(new Card("J", "6"));
    cards.append(new Card("J", "6"));
    cards.append(new Card("J", "7"));
    cards.append(new Card("J", "7"));
    cards.append(new Card("J", "8"));
    cards.append(new Card("J", "8"));
    cards.append(new Card("J", "9"));
    cards.append(new Card("J", "9"));
    cards.append(new Card("V", "0"));
    cards.append(new Card("V", "1"));
    cards.append(new Card("V", "1"));
    cards.append(new Card("V", "2"));
    cards.append(new Card("V", "2"));
    cards.append(new Card("V", "3"));
    cards.append(new Card("V", "3"));
    cards.append(new Card("V", "4"));
    cards.append(new Card("V", "4"));
    cards.append(new Card("V", "5"));
    cards.append(new Card("V", "5"));
    cards.append(new Card("V", "6"));
    cards.append(new Card("V", "6"));
    cards.append(new Card("V", "7"));
    cards.append(new Card("V", "7"));
    cards.append(new Card("V", "8"));
    cards.append(new Card("V", "8"));
    cards.append(new Card("V", "9"));
    cards.append(new Card("V", "9"));
    cards.append(new Card("R", "+2"));
    cards.append(new Card("R", "+2"));
    cards.append(new Card("B", "+2"));
    cards.append(new Card("B", "+2"));
    cards.append(new Card("J", "+2"));
    cards.append(new Card("J", "+2"));
    cards.append(new Card("V", "+2"));
    cards.append(new Card("V", "+2"));
    cards.append(new Card("R", "I"));
    cards.append(new Card("R", "I"));
    cards.append(new Card("B", "I"));
    cards.append(new Card("B", "I"));
    cards.append(new Card("J", "I"));
    cards.append(new Card("J", "I"));
    cards.append(new Card("V", "I"));
    cards.append(new Card("V", "I"));
    cards.append(new Card("R", "P"));
    cards.append(new Card("R", "P"));
    cards.append(new Card("B", "P"));
    cards.append(new Card("B", "P"));
    cards.append(new Card("J", "P"));
    cards.append(new Card("J", "P"));
    cards.append(new Card("V", "P"));
    cards.append(new Card("V", "P"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "+4"));
    cards.append(new Card("N", "J"));
    cards.append(new Card("N", "J"));
    cards.append(new Card("N", "J"));
    cards.append(new Card("N", "J"));
}

Card* Cards::get(int i) const
{
    return cards.at(i);
}

int Cards::size() const
{
    return cards.size();
}

Card* Cards::first() const
{
    return cards.first();
}

Card* Cards::last() const
{
    return cards.last();
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

bool Cards::isEmpty() const
{
    return cards.isEmpty();
}
