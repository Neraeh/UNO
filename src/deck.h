#ifndef DECK_H
#define DECK_H

#include "uno.h"
#include "card.h"

QT_FORWARD_DECLARE_CLASS(UNO)
class Deck
{
public:
    Deck(UNO *_parent);
    void init();
    QString randCards(int _count, bool colored = true);
    void remCard(Color _color, QString _id);

    QList<Card*> getList() const
    {
        return cards;
    }

    int size() const
    {
        return cards.size();
    }

    bool isEmpty() const
    {
        return cards.isEmpty();
    }

    bool contains(Card *_card) const;
    bool containsColor(Color _color) const;
    bool containsId(QString _id) const;
    QString toString(bool colored = true) const;

private:
    UNO *parent;
    QList<Card*> cards;
};

#endif // DECK_H
