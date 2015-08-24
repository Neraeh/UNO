#ifndef USERS_H
#define USERS_H

#include "user.h"

class Users : public QObject
{
    Q_OBJECT
public:
    explicit Users();
    void add(User* _user);
    void remove(QString _nick);
    void remove(User* _user);
    bool contains(QString _nick) const;
    User* get(QString _nick) const;
    int indexOf(QString _nick) const;
    QList<User*> getList() const;

private:
    QList<User*> users;
};

#endif // USERS_H
