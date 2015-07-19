#ifndef PLAYERS_H
#define PLAYERS_H

#include <QObject>
#include <QList>
#include "player.h"

class Players : public QObject
{
    Q_OBJECT
public:
    explicit Players();
    void add(Player* _player);
    void remove(QString _name);
    bool contains(QString _name) const;
    QString toString() const;
    Player *rand() const;
    Player* getPlayer(QString _name) const;
    Player* getPlayer(int i) const;
    int indexOf(QString _name) const;
    int size() const;
    bool isInversed() const;
    void clear();

private:
    QList<Player*> players;
    bool inversed;
};

#endif // PLAYERS_H
