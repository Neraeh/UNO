#ifndef USER_H
#define USER_H

#include <QString>
#include <QTime>

class User
{
public:
    User(QString _nick, unsigned short _color, QString _mode = QString(), bool _colored = true);
    User(QString _nick, QString _mode = QString());

    inline QString getNick() const
    {
        return nick;
    }

    inline QString getMode() const
    {
        return mode;
    }

    inline QString getHostname() const
    {
        return hostname;
    }

    inline bool getColored() const
    {
        return colored;
    }

    inline unsigned short getColor() const
    {
        return color;
    }

    inline QString getColoredName() const
    {
        return "\x03" + QString(color < 10 ? "0" : "") + QString::number(color) + ",14" + nick + "\x03""00,14";
    }

    inline void setNick(QString _nick)
    {
        nick = _nick;
    }

    inline void setMode(QString _mode)
    {
        mode = _mode;
    }

    inline void setHostname(QString _hostname)
    {
        hostname = _hostname;
    }

    inline void setColored(bool _colored)
    {
        colored = _colored;
    }

    inline void setColor(unsigned short _color)
    {
        color = _color;
    }

private:
    QString nick, mode, hostname;
    bool colored;
    unsigned short color;
};

#endif // USER_H
