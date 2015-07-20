#ifndef ECHOS_H
#define ECHOS_H

#include <QCoreApplication>
#include <QTextCodec>
#include <QLinkedList>
#include <IrcConnection>
#include <IrcCommand>
#include <Irc>
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
    void onNick(IrcNickMessage *message);
    void onNotice(IrcNoticeMessage *message);
    void onPart(IrcPartMessage *message);
    void onQuit(IrcQuitMessage *message);

private:
    void showCards(QString nick = QString());
    QString nextPlayer();
    void remPlayer(QString nick);
    void clear();
    void sendMessageIG(QString message);
    void sendMessage(QString message);
    void command(QString cmd, QStringList args);

private:
    QCoreApplication* parent;
    Cards *cards;
    Players *players;
    QList<QString> turns;
    Card *lastCard;
    QString currPlayer, currPing, chan;
    bool inGame, preGame, drawed, inversed, inPing;
    unsigned int pingTimeBegin, pingTime, pingCount;
};

#endif // ECHOS_H
