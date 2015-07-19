#include "echos.h"

Echos::Echos(QObject *parent) : QObject(parent)
{
    cards = new Cards;
}

Cards* Echos::getCards() const
{
    return cards;
}
