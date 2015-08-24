#include "players.h"

Players::Players()
{
    inversed = false;
}

void Players::add(Player* _player)
{
    players.append(_player);
}

void Players::remove(QString _name)
{
    players.removeAt(indexOf(_name));
}

void Players::remove(Player *_player)
{
    players.removeOne(_player);
}

bool Players::contains(QString _name) const
{
    foreach (Player* w, players)
        if (w->getName() == _name)
            return true;
    return false;
}

QString Players::list() const
{
    QString str;
    foreach (Player* w, players)
        str += w->getColoredName() + ", ";
    str.chop(2);
    return str;
}

QString Players::toString() const
{
    QString str;
    foreach (Player* w, players)
        str += w->getColoredName() + ", ";
    str.chop(2);
    return str;
}

QString Players::first() const
{
    return players.at(0)->getName();
}

Player* Players::rand() const
{
    qsrand(QTime::currentTime().msec());
    return players.at(qrand() % players.size());
}

Player* Players::get(QString _name) const
{
    foreach (Player* w, players)
        if (w->getName() == _name)
            return w;
    return NULL;
}

int Players::indexOf(QString _name) const
{
    foreach (Player* w, players)
        if (w->getName() == _name)
            return players.indexOf(w);
    return -1;
}

int Players::size() const
{
    return players.size();
}

bool Players::isEmpty() const
{
    return players.size() - 1 == 0 ? true : false;
}

bool Players::isInversed() const
{
    return inversed;
}

void Players::clear()
{
    players.clear();
    inversed = false;
}

QList<Player*> Players::getList() const
{
    return players;
}
