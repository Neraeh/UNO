#ifndef CARDS_H
#define CARDS_H

#include "card.h"

class Cards : public QObject
{
    Q_OBJECT
public:
    explicit Cards();
    Card* get(int i) const;
    int size() const;
    void remove(int i);
    bool isEmpty() const;

private:
    QList<Card*> cards;
};

#endif // CARDS_H
