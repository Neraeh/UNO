#ifndef IRC
#define IRC

#include <QtNetwork/QTcpSocket>
#include <QTimer>
#include <QList>
#include "chan.h"

class Chan;
class Irc : public QObject
{
    Q_OBJECT

public:
    explicit Irc();
    ~Irc();
    bool isConnected();
    QList<Chan*> *getChans();
    QString getNick();

public slots:
    void connect(const QString _nick, const QString _server, const unsigned long int _port, const QString _realname);
    void disconnect();
    void join(QString _chan);
    void part(QString _chan);
    void topic(QString _chan);
    void setNick(QString _nick);
    void sendMessage(QString _chan, QString _message);
    void sendNotice(QString _target, QString _notice);
    void sendAction(QString _target, QString _action);

private:
    QTcpSocket *socket;
    QString nick, server, realname;
    unsigned long int port;
    QList<Chan*> chans;

private slots:
    void readData();
    void connectToServer(QAbstractSocket::SocketError _error = QAbstractSocket::UnknownSocketError);
    void joinChans();
    void disconnectFromServer();

signals:
    void debugOutput(QString);
    void networkOutput(QString);
    void onMessage(QString, QString, QString);
    void onJoin(QString, QString, QString, QString);
    void usersChange(QString, QStringList);
    void topicChanged(QString, QString);
};

#endif // IRC

