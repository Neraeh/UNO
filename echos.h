#ifndef ECHOS_H
#define ECHOS_H

#include "irc.h"

class Echos : public QObject
{
    Q_OBJECT
public:
    explicit Echos(QObject *parent = 0);

signals:

public slots:
};

#endif // ECHOS_H
