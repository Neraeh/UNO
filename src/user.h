#ifndef USER_H
#define USER_H

#include <QString>

class User
{
public:
    explicit User(QString _nick, QString _mode = QString());
    QString getNick() const;
    QString getMode() const;
    bool getColor() const;
    void setNick(QString _nick);
    void setMode(QString _mode);
    void setColor(bool _color);

private:
    QString nick, mode;
    bool color;
};

#endif // USER_H
