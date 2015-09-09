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

class Players;
class Cards;
class UNO : public IrcConnection
{
    Q_OBJECT
public:
    explicit UNO(QCoreApplication *_parent = 0);
    ~UNO();
    Cards* getCards() const;
    Users* getUsers() const;
    Players* getPlayers() const;

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
    QSettings *settings, *slaps, *colors, *scores, *bans;
};

#endif // UNO_H

