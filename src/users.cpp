#include "users.h"

Users::Users() {}

void Users::add(User* _user)
{
    if (this->contains(_user->getNick()))
    {
        _user->setColor(this->get(_user->getNick())->getColor());
        _user->setColored(this->get(_user->getNick())->getColored());
        this->remove(_user->getNick());
    }

    users.append(_user);
}

bool Users::contains(QString _nick) const
{
    foreach (User* w, users)
        if (w->getNick() == _nick)
            return true;
    return false;
}

User* Users::get(QString _nick) const
{
    foreach (User* w, users)
        if (w->getNick() == _nick)
            return w;
    return new User(_nick, QString());
}

int Users::indexOf(QString _nick) const
{
    foreach (User* w, users)
        if (w->getNick() == _nick)
            return users.indexOf(w);
    return -1;
}
