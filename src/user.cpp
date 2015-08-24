#include "user.h"

User::User(QString _nick, QString _mode)
{
    nick = _nick, mode = _mode, color = true;
}

QString User::getNick() const
{
    return nick;
}

QString User::getMode() const
{
    return mode;
}

bool User::getColor() const
{
    return color;
}

void User::setNick(QString _nick)
{
    nick = _nick;
}

void User::setMode(QString _mode)
{
    mode = _mode;
}

void User::setColor(bool _color)
{
    color = _color;
}
