#ifndef PLAYER_H
#define PLAYER_H

#include "uno.h"
#include "deck.h"

class Deck;
class UNO;
class Player
{
public:
    Player(QString _name, Deck *_deck, bool _canPlay, QString _color);
    Player(QString _name, UNO *_parent);

    inline Deck* getDeck() const
    {
        return deck;
    }

    inline QString getName() const
    {
        return name;
    }

    inline QString getColoredName() const
    {
        return color + name + "\x03""00,14";
    }

    inline QString getColor() const
    {
        return color;
    }

    bool canPlay();

    inline void cantPlay()
    {
        play = false;
    }

private:
    QString name, color;
    Deck* deck;
    bool play;
};

#endif // PLAYER_H
