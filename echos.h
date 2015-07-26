#ifndef ECHOS_H
#define ECHOS_H

#include <QCoreApplication>
#include <QTextCodec>
#include <IrcConnection>
#include <IrcCommand>
#include <QSettings>
#include <QTimer>
#include "cards.h"
#include "players.h"

class Players;
class Echos : public IrcConnection
{
    Q_OBJECT
public:
    explicit Echos(QCoreApplication *_parent = 0);
    ~Echos();
    Cards* getCards() const;

public slots:
    void onConnect();
    void onMessage(IrcPrivateMessage *message);
    void onJoin(IrcJoinMessage *message);
    void onKick(IrcKickMessage *message);
    void onMode(IrcModeMessage *message);
    void onNames(IrcNamesMessage *message);
    void onNick(IrcNickMessage *message);
    void onNotice(IrcNoticeMessage *message);
    void onPart(IrcPartMessage *message);
    void onQuit(IrcQuitMessage *message);

private slots:
    void pingTimeout();
    void versionTimeout(QString nick);

private:
    void showCards(QString nick = QString(), QString to = QString());
    QString nextPlayer();
    void remPlayer(QString nick);
    void clear();
    void sendMessage(QString message);
    void command(QString nick, QString cmd, QStringList args);
    bool isOp(QString user);
    bool startsWithMode(QString nick);

private:
    Cards *cards;
    Players *players;
    QList<QString> turns;
    Card *lastCard;
    QString currPlayer, currPing, chan;
    QHash<QString,QString> modes;
    bool inGame, preGame, drawed, inversed, inPing, inVersion;
    unsigned int pingTimeBegin, pingTime, pingCount;
    QSettings* slaps;
};

#endif // ECHOS_H

