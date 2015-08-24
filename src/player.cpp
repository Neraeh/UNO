#include "player.h"

Player::Player(QString _name, Deck* _deck, bool _canPlay, QString _color)
{
    name = _name, deck = _deck, play = _canPlay, color = _color;
}

Player::Player(QString _name, UNO *_parent)
{
    name = _name;
    qsrand(QTime::currentTime().msec());
    int rand = (qrand() % (13 - 2) + 1) + 2;
    color = "\x03" + QString(rand < 10 ? "0" : "") + QString::number(rand) + ",14";
    deck = new Deck(_parent);
    play = true;
}

Deck* Player::getDeck() const
{
    return deck;
}

QString Player::getName() const
{
    return name;
}

QString Player::getColoredName() const
{
    return color + name + "\x03""00,14";
}

QString Player::getColor() const
{
    return color;
}

bool Player::canPlay()
{
    if (play)
        return true;
    play = true;
    return false;
}

void Player::cantPlay()
{
    play = false;
}
