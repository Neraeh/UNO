#include "deck.h"

Deck::Deck(UNO *_parent)
{
    parent = _parent;
}

void Deck::init()
{
    while (cards.size() < 7)
        cards.append(parent->getCards()->pick());
}

QString Deck::randCards(int _count, bool colored)
{
    QString newCards = "+ ";
    for (int i = 0; i < _count; i++)
    {
        Card* newCard = parent->getCards()->pick();
        cards.append(newCard);
        newCards += newCard->toString(colored) + " ";
    }
    return newCards;
}

void Deck::remCard(QString _color, QString _id)
{
    foreach (Card* w, cards)
    {
        if (w->getColor() == _color && w->getId() == _id)
        {
            cards.removeOne(w);
            return;
        }
    }
}

QList<Card*> Deck::getList() const
{
    return cards;
}

bool Deck::contains(Card* _card) const
{
    foreach (Card* w, cards)
        if (w->getId() == _card->getId() && w->getColor() == _card->getColor())
            return true;
    return false;
}

bool Deck::containsColor(QString _color) const
{
    foreach (Card* w, cards)
        if (w->getColor() == _color.toUpper())
            return true;
    return false;
}

bool Deck::containsId(QString _id) const
{
    foreach (Card* w, cards)
        if (w->getId() == _id.toUpper())
            return true;
    return false;
}

int Deck::size() const
{
    return cards.size();
}

bool Deck::isEmpty() const
{
    return cards.isEmpty();
}

QString Deck::toString(bool colored) const
{
    QString deck;
    foreach (Card* w, cards)
        deck += w->toString(colored) + " ";
    return deck;
}
