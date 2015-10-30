#ifndef UNO_H
#define UNO_H

#include <QCoreApplication>
#include <QTextCodec>
#include <IrcConnection>
#include <IrcCommand>
#include <QSettings>
#include <QTimer>
#include <QHash>
#include <QProcess>
#include "cards.h"
#include "players.h"
#include "users.h"
#include "card.h"
#include "player.h"
#include "commit_date.h"

class Players;
class Cards;
class UNO : public IrcConnection
{
    Q_OBJECT
public:
    UNO(QCoreApplication *_parent = 0);
    ~UNO();

    inline Cards* getCards() const
    {
        return pick;
    }

    inline Users* getUsers() const
    {
        return users;
    }

    inline Players* getPlayers() const
    {
        return players;
    }

private slots:
    void onSSLError();
    void onConnect();
    void onDisconnect();
    void onIrcMessage(IrcMessage *message);
    void onMessage(IrcPrivateMessage *message);
    void onJoin(IrcJoinMessage *message);
    void onKick(IrcKickMessage *message);
    void onMode(IrcModeMessage *message);
    void onNames(IrcNamesMessage *message);
    void onNick(IrcNickMessage *message);
    void onNotice(IrcNoticeMessage *message);
    void onPart(IrcPartMessage *message);
    void onQuit(IrcQuitMessage *message);

    void pingTimeout();
    void versionTimeout(QString nick);
    void preGameTimeout();

private:
    void showCards(QString nick = QString(), QString to = QString());
    QString nextPlayer() const;
    void remPlayer(QString nick);
    void clear();
    void sendNotice(QString target, QString message);
    void sendMessage(QString message, Card* card = 0);
    void flushMessages();
    void command(QString nick, QString cmd, QStringList args);
    QString showTurns() const;
    void showScores();
    bool isOp(QString user);
    bool startsWithMode(QString nick);

    inline void log(QString w, unsigned int level) // Level rules: 0 = nothing; 1 += errors and warnings only; 2 += infos; 3 += libcommuni debug
    {
        if (level <= verbose)
            qDebug() << qPrintable(w);
    }

private:
    Cards *pick;
    Players *players;
    QStringList turns, messages;
    QHash<QString,QString> notices;
    Card *lastCard;
    QString currPlayer, currPing, chan;
    Users *users;
    bool inGame, preGame, drawed, inversed, inPing, inVersion;
    unsigned int pingTimeBegin, pingTime, pingCount;
    QSettings *settings, *slaps, *colors, *scores, *bans, *accesslist;
    unsigned int verbose;
};

#endif // UNO_H

