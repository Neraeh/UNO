#ifndef PLAYERS_H
#define PLAYERS_H

#include <QObject>

class Players : public QObject
{
    Q_OBJECT
public:
    explicit Players(QObject *parent = 0);

signals:

public slots:
};

#endif // PLAYERS_H
