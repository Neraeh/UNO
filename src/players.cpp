#include "players.h"

Players::Players()
{
    inversed = false;
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
    return players.first()->getName();
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

void Players::clear()
{
    players.clear();
    inversed = false;
}
