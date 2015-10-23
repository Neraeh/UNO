#include "player.h"

Player::Player(QString _name, Deck* _deck, bool _canPlay, QString _color)
{
    name = _name, deck = _deck, play = _canPlay, color = _color;
}

Player::Player(QString _name, UNO *_parent)
{
    name = _name;
    color = "\x03" + QString(_parent->getUsers()->get(name)->getColor() < 10 ? "0" : "") + QString::number(_parent->getUsers()->get(name)->getColor()) + ",14";
    deck = new Deck(_parent);
    play = true;
}

bool Player::canPlay()
{
    if (play)
        return true;
    play = true;
    return false;
}
