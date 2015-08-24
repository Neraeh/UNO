#ifndef PLAYER_H
#define PLAYER_H

#include "uno.h"
#include "deck.h"

class Deck;
class UNO;
class Player
{
public:
    explicit Player(QString _name, Deck *_deck, bool _canPlay, QString _color);
    explicit Player(QString _name, UNO *_parent);
    Deck* getDeck() const;
    QString getName() const;
    QString getColoredName() const;
    QString getColor() const;
    bool canPlay();
    void cantPlay();

private:
    QString name, color;
    Deck* deck;
    bool play;
};

#endif // PLAYER_H
