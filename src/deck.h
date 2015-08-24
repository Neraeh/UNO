#ifndef DECK_H
#define DECK_H

#include "uno.h"
#include "card.h"

class UNO;
class Deck : public QObject
{
    Q_OBJECT
public:
    explicit Deck(UNO *_parent);
    void init();
    QString randCards(int _count, bool colored = true);
    void remCard(QString _color, QString _id);
    bool contains(Card *_card) const;
    bool containsColor(QString _color) const;
    bool containsId(QString _id) const;
    int size() const;
    bool isEmpty() const;
    QString toString(bool colored = true) const;

private:
    UNO *parent;
    QList<Card*> cards;
};

#endif // DECK_H
