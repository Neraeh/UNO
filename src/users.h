#ifndef USERS_H
#define USERS_H

#include <QList>
#include "user.h"

class Users
{
public:
    Users();
    void add(User* _user);

    inline void remove(QString _nick)
    {
        users.removeAt(indexOf(_nick));
    }

    inline void remove(User* _user)
    {
        users.removeOne(_user);
    }

    bool contains(QString _nick) const;
    User* get(QString _nick) const;
    int indexOf(QString _nick) const;

    inline QList<User*> getList() const
    {
        return users;
    }

private:
    QList<User*> users;
};

#endif // USERS_H
