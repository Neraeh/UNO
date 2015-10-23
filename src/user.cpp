#include "user.h"

User::User(QString _nick, unsigned short _color, QString _mode, bool _colored)
{
    nick = _nick, mode = _mode, colored = _colored, color = _color;
}

User::User(QString _nick, QString _mode)
{
    nick = _nick, mode = _mode, colored = true;
    color = (qrand() % (13 - 2) + 1) + 2;
}
