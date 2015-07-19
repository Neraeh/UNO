#include "chan.h"

Chan::Chan(Irc *_parent, QString _name) : QObject()
{
    name = _name;
    parent = _parent;
}

Chan::~Chan() {}

QString Chan::getName()
{
    return name;
}

void Chan::addUser(User *_user)
{
    userlist.append(_user);
}

QList<User*> Chan::getUserlist()
{
    return userlist;
}

void Chan::setTopic(QString _topic)
{
    topic = _topic;
}

QString Chan::getTopic()
{
    return topic;
}
