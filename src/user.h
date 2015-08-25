#ifndef USER_H
#define USER_H

#include <QString>
#include <QTime>

class User
{
public:
    explicit User(QString _nick, unsigned short _color, QString _mode = QString(), bool _colored = true);
    explicit User(QString _nick, QString _mode = QString());
    QString getNick() const;
    QString getMode() const;
    bool getColored() const;
    unsigned short getColor() const;
    QString getColoredName() const;
    void setNick(QString _nick);
    void setMode(QString _mode);
    void setColored(bool _colored);
    void setColor(unsigned short _color);

private:
    QString nick, mode;
    bool colored;
    unsigned short color;
};

#endif // USER_H
