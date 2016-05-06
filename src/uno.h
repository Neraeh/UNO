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
#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QThread>
#include "cards.h"
#include "players.h"
#include "users.h"
#include "card.h"
#include "player.h"
#include "updater.h"
#include "commit_date.h"

class Players;
class Cards;
class Updater;
class UNO : public IrcConnection
{
    Q_OBJECT
public:
    UNO(QCoreApplication *_parent = 0);
    ~UNO();

    enum Log {
        INFO,
        WARNING,
        ERROR,
        INIT
    };

    void log(Log l, QString w) // Level rules: 0 = nothing; 1 += errors and warnings only; 2 += infos; 3 += libcommuni debug
    {
        switch (l) {
        case INFO:
            if (verbose >= 2)
                qDebug() << qPrintable("[" + tr("INFO") + "] " + w);
            break;
        case WARNING:
            if (verbose >= 1)
                qDebug() << qPrintable("[" + tr("WARN") + "] " + w);
            break;
        case ERROR:
            if (verbose >= 1)
                qDebug() << qPrintable("[" + tr("ERROR") + "] " + w);
            break;
        case INIT:
            qDebug() << qPrintable(w);
        }
    }

    Cards* getCards() const
    {
        return pick;
    }

    Users* getUsers() const
    {
        return users;
    }

    Players* getPlayers() const
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

    void onUpdaterStep(QString step);
    void onUpdaterError(QString step);
    void onUpdaterDone();

    void pingTimeout();
    void versionTimeout();
    void preGameTimeout();

private:
    void showCards(QString nick = QString(), QString to = QString());
    QString nextPlayer() const;
    void remPlayer(QString nick);
    void clear();
    void sendNotice(QString target, QString message);
    void sendMessage(QString message, Card* card = NULL, bool direct = false);
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
    QString currPlayer, currPing, currVersion, chan;
    Users *users;
    bool inGame, preGame, drawed, inversed, inPing, inVersion;
    unsigned int pingTimeBegin, pingTime, pingCount;
    QSettings *settings, *slaps, *colors, *scores, *bans, *accesslist;
    unsigned int verbose;
    Updater *updater;
};

#endif // UNO_H

