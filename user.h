#ifndef USER_H
#define USER_H

#include <QObject>

class User : public QObject
{
    Q_OBJECT

public:
    explicit User(QString _nick, QString _realname, QString _host, QString _mode);
    explicit User(QString _nick);
    ~User();
    QString getNick();
    QString get(QString _id);
    void set(QString _id, QString _content);
    bool isFilled();

private:
    QString nick, ip, realname, host, mode;
    bool filled;
};

#endif // USER_H
