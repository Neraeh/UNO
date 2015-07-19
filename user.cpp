#include "user.h"

User::User(QString _nick, QString _realname = "", QString _host = "", QString _mode = "") : QObject()
{
    nick = _nick, realname = _realname, host = _host, mode = _mode;
    filled = true;
}

User::User(QString _nick)
{
    nick = _nick;
    filled = false;
}

User::~User() {}

QString User::get(QString _id)
{
    if (_id == "nick")
        return nick;
    else if (_id == "realname")
        return realname;
    else if (_id == "host")
        return host;
    else if (_id == "mode")
        return mode;
    else
        return "";
}

void User::set(QString _id, QString _content)
{
    if (_id == "nick")
        nick = _content;
    else if (_id == "realname")
        realname = _content;
    else if (_id == "host")
        host = _content;
    else if (_id == "mode")
        mode = _content;

    if (nick != "" && realname != "" && host != "" && !filled)
        filled = true;
}

bool User::isFilled()
{
    return filled;
}
