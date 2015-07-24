#ifndef PLAYERS_H
#define PLAYERS_H

#include <QObject>
#include <QList>
#include "player.h"

class Player;
class Players : public QObject
{
    Q_OBJECT
public:
    explicit Players();
    void add(Player* _player);
    void remove(QString _name);
    void remove(Player* _player);
    bool contains(QString _name) const;
    QString list() const;
    QString toString() const;
    QString first() const;
    Player *rand() const;
    Player* getPlayer(QString _name) const;
    int indexOf(QString _name) const;
    int size() const;
    bool isInversed() const;
    void clear();
    QList<Player*> getList() const;

private:
    QList<Player*> players;
    bool inversed;
};

#endif // PLAYERS_H
