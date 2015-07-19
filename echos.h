#ifndef ECHOS_H
#define ECHOS_H

#include <QCoreApplication>
#include <IrcConnection>
#include <IrcCommand>
#include <Irc>
#include "irc.h"
#include "cards.h"

class Echos : public IrcConnection
{
    Q_OBJECT
public:
    explicit Echos(QCoreApplication *parent = 0);
    Cards *getCards() const;
signals:

public slots:

private:
    Cards *cards;
};

#endif // ECHOS_H
