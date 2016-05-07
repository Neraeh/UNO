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

void Deck::remCard(Color _color, QString _id)
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

bool Deck::contains(Card* _card) const
{
    foreach (Card* w, cards)
        if (w->getId() == _card->getId() && w->getColor() == _card->getColor())
            return true;
    return false;
}

bool Deck::containsColor(Color _color) const
{
    foreach (Card* w, cards)
        if (w->getColor() == _color)
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

QString Deck::toString(bool colored) const
{
    QMultiMap<int,Card*> red;
    QMultiMap<int,Card*> green;
    QMultiMap<int,Card*> blue;
    QMultiMap<int,Card*> yellow;
    QMultiMap<int,Card*> none;
    bool ok;
    int nbr;

    foreach (Card* w, cards)
    {
        nbr = w->getId().toInt(&ok, 10);
        if (ok)
        {
            switch (w->getColor())
            {
            case RED:
                red.insert(nbr, w);
                break;
            case GREEN:
                green.insert(nbr, w);
                break;
            case BLUE:
                blue.insert(nbr, w);
                break;
            case YELLOW:
                yellow.insert(nbr, w);
                break;
            default:
                none.insert(1, w);
            }
        }
        else
        {
            switch (w->getColor())
            {
            case RED:
                red.insert(-1, w);
                break;
            case GREEN:
                green.insert(-1, w);
                break;
            case BLUE:
                blue.insert(-1, w);
                break;
            case YELLOW:
                yellow.insert(-1, w);
                break;
            default:
                none.insert(2, w);
            }
        }
    }

    QString deck;
    foreach (Card* w, none.values() + red.values() + green.values() + blue.values() + yellow.values())
        deck += w->toString(colored) + " ";
    return deck;
}
