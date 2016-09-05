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

typedef void (UNO::*fp)(QString, QStringList);

QT_FORWARD_DECLARE_CLASS(Players)
QT_FORWARD_DECLARE_CLASS(Cards)
QT_FORWARD_DECLARE_CLASS(Updater)
class UNO : public IrcConnection
{
    Q_OBJECT
public:
    UNO(QCoreApplication *_parent = 0);
    ~UNO();

    enum Log
    {
        INFO,
        WARNING,
        ERROR,
        INIT
    };

    void log(Log l, QString w) // Level rules: 0 = errors only; 1 += warnings; 2 += infos; 3 += libcommuni debug
    {
        switch (l)
        {
        case INFO:
            if (verbose >= 2)
                qDebug() << qPrintable("[" + tr("INFO") + "] " + w);
            break;
        case WARNING:
            if (verbose >= 1)
                qDebug() << qPrintable("[" + tr("WARN") + "] " + w);
            break;
        case ERROR:
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
    void sendRawNotice(QString target, QString message);
    void sendNotice(QString target, QString message, Card *card = NULL);
    void sendMessage(QString message, Card* card = NULL, bool direct = false);
    void flushMessages();
    void command(QString nick, QString cmd, QStringList args);
    QString showTurns() const;
    void showScores();
    bool isOp(QString user);
    bool startsWithMode(QString nick);

// Commands
    void exit(QString nick, QStringList args);
#ifndef Q_OS_WIN
    void update(QString nick, QStringList args);
#endif
    void changeTrigger(QString nick, QStringList args);
    void kick(QString nick, QStringList args);
    void ban(QString nick, QStringList args);
    void unban(QString nick, QStringList args);
    void al(QString nick, QStringList args);
    void merge(QString nick, QStringList args);
    void color(QString nick, QStringList args);
    void nocolor(QString nick, QStringList args);
    void scores(QString nick, QStringList args);
    void help(QString nick, QStringList args);
    void rules(QString nick, QStringList args);
    void version(QString nick, QStringList args);
    void slap(QString nick, QStringList args);
    void ping(QString nick, QStringList args);
    void list(QString nick, QStringList args);
    void quitGame(QString nick, QStringList args);
    void joinGame(QString nick, QStringList args);
    void beginGame(QString nick, QStringList args);
    void uno(QString nick, QStringList args);
    void draw(QString nick, QStringList args);
    void endTurn(QString nick, QStringList args);
    void hand(QString nick, QStringList args);
    void cardsGame(QString nick, QStringList args);
    void play(QString nick, QStringList args);

    void turnEnd();

private:
    QHash<QString,fp> *commands;
    Cards *pick;
    Players *players;
    QStringList turns, messages, output;
    QMultiMap<QString,QString> notices;
    Card *lastCard;
    QString currPlayer, currPing, currVersion, chan, trigger;
    Users *users;
    bool rbash, cnf, identified, inGame, preGame, drawed, inversed, inPing, inVersion;
    unsigned int pingTimeBegin, pingTime, pingCount;
    QSettings *settings, *slaps, *colors, *score, *bans, *accesslist;
    unsigned int verbose;
    Updater *updater;
};

#endif // UNO_H
