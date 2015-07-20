#include "echos.h"

Echos::Echos(QCoreApplication *_parent) : IrcConnection()
{
    parent = _parent;
    chan = "#dev";
    cards = new Cards;
    players = new Players;
    lastCard = new Card("", "");
    inGame = false, preGame = false, drawed = false, inversed = false, inPing = false;

    qputenv("IRC_DEBUG", "1");

    setServers(QStringList("irc.t411.io"));
    setEncoding("UTF-8");
    setUserName("EchosTest");
    setNickName("EchosTest");
    setRealName("Echos");
    setReconnectDelay(5);
    open();

    QObject::connect(this, SIGNAL(connected()), this, SLOT(onConnect()));
    QObject::connect(this, SIGNAL(privateMessageReceived(IrcPrivateMessage*)), this, SLOT(onMessage(IrcPrivateMessage*)));
    QObject::connect(this, SIGNAL(joinMessageReceived(IrcJoinMessage*)), this, SLOT(onJoin(IrcJoinMessage*)));
    QObject::connect(this, SIGNAL(kickMessageReceived(IrcKickMessage*)), this, SLOT(onKick(IrcKickMessage*)));
    QObject::connect(this, SIGNAL(nickMessageReceived(IrcNickMessage*)), this, SLOT(onNick(IrcNickMessage*)));
    QObject::connect(this, SIGNAL(noticeMessageReceived(IrcNoticeMessage*)), this, SLOT(onNotice(IrcNoticeMessage*)));
    QObject::connect(this, SIGNAL(partMessageReceived(IrcPartMessage*)), this, SLOT(onPart(IrcPartMessage*)));
    QObject::connect(this, SIGNAL(quitMessageReceived(IrcQuitMessage*)), this, SLOT(onQuit(IrcQuitMessage*)));
}

Echos::~Echos()
{
    if (isActive())
    {
        quit("Closed");
        close();
    }
}

void Echos::onConnect()
{
    sendCommand(IrcCommand::createMessage("NickServ", "IDENTIFY GC1066echos&"));
    sendCommand(IrcCommand::createJoin(chan));
}

void Echos::onMessage(IrcPrivateMessage *message)
{
    if (message->target() == nickName() || message->isOwn())
        return;
    if (message->content().startsWith("!"))
    {
        QStringList args = message->content().split(" ", QString::SkipEmptyParts);
        args.removeFirst();
        command(message->content().split(" ", QString::SkipEmptyParts).first().mid(1), args);
    }
}

void Echos::onJoin(IrcJoinMessage *message)
{
    sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + message->nick()));
    if (preGame)
        sendMessage("Envie de jouer au UNO ? Tapez ""\x02""UNO""\x0F"" pour rejoindre la partie en préparation !");
    else if (!inGame)
        sendMessage("Envie de jouer au UNO ? Tapez ""\x02""!aide""\x0F"" pour afficher la liste des commandes");
}

void Echos::onKick(IrcKickMessage *message)
{
    if (message->user() == nickName())
        sendCommand(IrcCommand::createJoin(chan));
    else
        remPlayer(message->user());
}

void Echos::onNick(IrcNickMessage *message)
{
    sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + message->newNick()));
    if ((inGame || preGame) && players->contains(message->oldNick()))
    {
        Player *player = players->getPlayer(message->oldNick());
        players->add(new Player(message->newNick(), player->getDeck(), player->canPlay(), player->getColor()));
        players->remove(player);
        turns.replace(turns.indexOf(message->oldNick()), message->newNick());
        if (currPlayer == message->oldNick())
            currPlayer = message->newNick();
    }
}

void Echos::onNotice(IrcNoticeMessage *message)
{
    if (message->content().split(" ").size() > 3 && message->nick() == "NickServ" && message->content().startsWith("STATUS") && (message->content().split(" ").at(3) == "Shayy" || message->content().split(" ").at(3) == "TuxAnge" || message->content().split(" ").at(3) == "Feeling"))
    {
        sendCommand(IrcCommand::createMode(chan, "+o", message->content().split(" ").at(3)));
        return;
    }

    if (message->content().startsWith("\u0001""VERSION"))
    {
        sendMessage("\x02" + message->nick() + "\x0F"" utilise : " + message->content().mid(message->content().indexOf(" ")));
        return;
    }

    if (message->nick() == currPing && inPing)
    {
        if (QTime::currentTime().msec() - pingTimeBegin < pingTime)
            pingTime = QTime::currentTime().msec() - pingTimeBegin;
        pingCount++;

        if (pingCount < 3)
        {
            pingTimeBegin = QTime::currentTime().msec();
            sendCommand(IrcCommand::createCtcpAction(currPing, "PING " + pingTimeBegin));
            return;
        }
        else
        {
            int lg = (int)(pingTime / 100);
            QString red;
            for (int i = 1; i < lg; i++)
                red.append(" ");
            QString green;
            for (int i = 1; i < (50 - lg); i++)
                green.append(" ");
            sendMessage("\x03""01,03" + green + "\x03""01,04" + red + "\x0F"" ""\x02" + currPing + "\x0F"": " + pingTime + "ms");
            currPing = "";
            pingTime = 10000000;
            pingCount = 0;
            inPing = false;
        }
    }
}

void Echos::onPart(IrcPartMessage *message)
{
    remPlayer(message->nick());
}

void Echos::onQuit(IrcQuitMessage *message)
{
    remPlayer(message->nick());
}

void Echos::showCards(QString nick)
{
    if (nick.isEmpty())
        nick = currPlayer;
    sendCommand(IrcCommand::createNotice(nick, players->getPlayer(nick)->getDeck()->toString()));
}

QString Echos::nextPlayer()
{
    if (inversed)
    {
        if (turns.indexOf(currPlayer) == 0)
            return players->getPlayer(turns.at(turns.size() - 1))->getName();
        else
            return players->getPlayer(turns.at(turns.indexOf(currPlayer) - 1))->getName();
    }
    else
    {
        if (turns.indexOf(currPlayer) == turns.size() - 1)
            return players->getPlayer(turns.at(0))->getName();
        else
            return players->getPlayer(turns.at(turns.indexOf(currPlayer) + 1))->getName();
    }
}

void Echos::remPlayer(QString nick)
{
    if (players->contains(nick))
    {
        sendMessageIG(players->getPlayer(nick)->getColoredName() + " a quitté la partie");

        if (players->size() == 1 && inGame)
        {
            sendMessageIG(players->getPlayer(players->first())->getColoredName() + " a gagné la partie !");
            clear();
        }
        else if (nick == currPlayer)
        {
            currPlayer = nextPlayer();
            sendMessageIG("C'est donc au tour de " + players->getPlayer(currPlayer)->getColoredName());
        }
        else if (players->size() == 0 && preGame)
        {
            sendMessageIG("Plus aucun joueur, la préparation de la partie est annulée");
            clear();
        }

        players->remove(nick);
        turns.removeOne(nick);
    }
}

void Echos::clear()
{
    players->clear();
    turns.clear();
    inGame = false;
    preGame = false;
    drawed = false;
    inversed = false;
    cards = new Cards();
}

void Echos::sendMessageIG(QString message)
{
    if (inGame || preGame)
        sendMessage("\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14" + message + " ");
    else
        sendMessage(message);
}

void Echos::sendMessage(QString message)
{
    sendCommand(IrcCommand::createMessage(chan, message));
}

void Echos::command(QString cmd, QStringList args)
{
    // Handle des commandes des utilisateurs
}

Cards* Echos::getCards() const
{
    return cards;
}
