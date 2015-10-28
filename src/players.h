#ifndef PLAYERS_H
#define PLAYERS_H

#include "uno.h"

class UNO;
class Player;
class Players
{
public:
    Players(UNO* _parent);

    inline void add(Player* _player)
    {
        players.append(_player);
    }

    inline void remove(QString _name)
    {
        players.removeAt(indexOf(_name));
    }

    inline void remove(Player *_player)
    {
        players.removeOne(_player);
    }

    bool contains(QString _name) const;
    QString list() const;
    QString toString() const;
    QString first() const;

    inline Player* rand() const
    {
        return players.at(qrand() % players.size());
    }

    Player* get(QString _name) const;
    int indexOf(QString _name) const;

    inline int size() const
    {
        return players.size();
    }

    inline bool isEmpty() const
    {
        return players.size() - 1 == 0 ? true : false;
    }

    inline bool isInversed() const
    {
        return inversed;
    }

    void clear();

    inline QList<Player*> getList() const
    {
        return players;
    }

private:
    UNO* parent;
    QList<Player*> players;
    bool inversed;
};

#endif // PLAYERS_H

