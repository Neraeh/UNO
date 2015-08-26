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

void Users::remove(QString _nick)
{
    users.removeAt(indexOf(_nick));
}

void Users::remove(User* _user)
{
    users.removeOne(_user);
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
    return NULL;
}

int Users::indexOf(QString _nick) const
{
    foreach (User* w, users)
        if (w->getNick() == _nick)
            return users.indexOf(w);
    return -1;
}

QList<User*> Users::getList() const
{
    return users;
}
