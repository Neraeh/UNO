#ifndef ECHOS_H
#define ECHOS_H

#include "irc.h"
#include "cards.h"

class Echos : public QObject
{
    Q_OBJECT
public:
    explicit Echos(QObject *parent = 0);
    Cards *getCards() const;
signals:

public slots:

private:
    Cards *cards;
};

#endif // ECHOS_H
