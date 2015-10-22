#include "uno.h"
#include "commit_date.h"

UNO::UNO(QCoreApplication *_parent) : IrcConnection(_parent)
{
    qsrand(QTime::currentTime().msec());

    pick = new Cards(this);
    players = new Players;
    users = new Users;
    lastCard = new Card("", "");
    inGame = false, preGame = false, drawed = false, inversed = false, inPing = false, inVersion = false;

    settings = new QSettings(qApp->applicationDirPath() + "/UNObox/settings.ini", QSettings::IniFormat);
    settings->setIniCodec("UTF-8");

    slaps = new QSettings(qApp->applicationDirPath() + "/UNObox/slaps.ini", QSettings::IniFormat);
    slaps->setIniCodec("UTF-8");

    colors = new QSettings(qApp->applicationDirPath() + "/UNObox/colors.ini", QSettings::IniFormat);
    colors->setIniCodec("UTF-8");

    scores = new QSettings(qApp->applicationDirPath() + "/UNObox/scores.ini", QSettings::IniFormat);
    scores->setIniCodec("UTF-8");

    bans = new QSettings(qApp->applicationDirPath() + "/UNObox/bans.ini", QSettings::IniFormat);
    scores->setIniCodec("UTF-8");

    accesslist = new QSettings(qApp->applicationDirPath() + "/UNObox/accesslist.ini", QSettings::IniFormat);
    accesslist->setIniCodec("UTF-8");

    qputenv("IRC_DEBUG", settings->value("debug", "0").toByteArray());

    setHost(settings->value("server", "irc.freenode.net").toString());
    setPort(settings->value("port", 6667).toInt());
    setSecure(settings->value("ssl", false).toBool());
    chan = (settings->value("chan", "##newuno").toString().startsWith("#") ? "" : "#") + settings->value("chan", "##newuno").toString();
    setEncoding(settings->value("encoding", "UTF-8").toByteArray());
    setUserName(settings->value("username", "UNO").toString());
    setNickName(settings->value("nickname", "UNO").toString());
    setRealName(settings->value("realname", "UNO").toString());
    setReconnectDelay(5);

    QVariantMap CtcpReplies;
    CtcpReplies.insert("VERSION", "VERSION UNO [Update "COMMITDATE"]");
    CtcpReplies.insert("SOURCE", "SOURCE https://github.com/TheShayy/UNO");
    setCtcpReplies(CtcpReplies);

    open();

    QObject::connect(this, SIGNAL(connected()), this, SLOT(onConnect()));
    QObject::connect(this, SIGNAL(privateMessageReceived(IrcPrivateMessage*)), this, SLOT(onMessage(IrcPrivateMessage*)));
    QObject::connect(this, SIGNAL(joinMessageReceived(IrcJoinMessage*)), this, SLOT(onJoin(IrcJoinMessage*)));
    QObject::connect(this, SIGNAL(kickMessageReceived(IrcKickMessage*)), this, SLOT(onKick(IrcKickMessage*)));
    QObject::connect(this, SIGNAL(modeMessageReceived(IrcModeMessage*)), this, SLOT(onMode(IrcModeMessage*)));
    QObject::connect(this, SIGNAL(namesMessageReceived(IrcNamesMessage*)), this, SLOT(onNames(IrcNamesMessage*)));
    QObject::connect(this, SIGNAL(nickMessageReceived(IrcNickMessage*)), this, SLOT(onNick(IrcNickMessage*)));
    QObject::connect(this, SIGNAL(noticeMessageReceived(IrcNoticeMessage*)), this, SLOT(onNotice(IrcNoticeMessage*)));
    QObject::connect(this, SIGNAL(partMessageReceived(IrcPartMessage*)), this, SLOT(onPart(IrcPartMessage*)));
    QObject::connect(this, SIGNAL(quitMessageReceived(IrcQuitMessage*)), this, SLOT(onQuit(IrcQuitMessage*)));
}

UNO::~UNO()
{
    if (isActive())
    {
        quit("Closed");
        close();
    }
    settings->sync();
    slaps->sync();
    colors->sync();
    scores->sync();
    delete pick;
    delete players;
    delete users;
    delete lastCard;
    delete settings;
    delete slaps;
    delete colors;
    delete scores;
    delete bans;
}

void UNO::onConnect()
{
    if (!settings->value("nspassword", QString()).toString().isEmpty())
        sendCommand(IrcCommand::createMessage("NickServ", "IDENTIFY " + settings->value("nspassword").toString()));
    else
        sendCommand(IrcCommand::createJoin(chan));
}

void UNO::onMessage(IrcPrivateMessage *message)
{
    users->get(message->nick())->setHostname(message->host());
    foreach (QString w, bans->allKeys())
        if (bans->value(w) == message->host())
            return;

    if (message->target() == nickName() || message->isOwn())
        return;
    else if (bans->allKeys().contains(message->nick()))
        return;
    else if (message->content().startsWith("!"))
    {
        QStringList args = message->content().split(" ", QString::SkipEmptyParts);
        args.removeFirst();
        command(message->nick(), message->content().split(" ", QString::SkipEmptyParts).first().mid(1), args);
    }
}

void UNO::onJoin(IrcJoinMessage *message)
{
    sendCommand(IrcCommand::createNames(chan));
    sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + message->nick()));
    if (preGame)
        sendMessage("Wanna play? Try the ""\x02""!join""\x0F"" command!");
    else if (!inGame)
        sendMessage("Wanna play? Try the ""\x02""!help""\x0F"" command!");
    flushMessages();
}

void UNO::onKick(IrcKickMessage *message)
{
    if (message->user() == nickName())
        sendCommand(IrcCommand::createJoin(chan));
    else
        remPlayer(message->user());
}

void UNO::onMode(IrcModeMessage *message)
{
    if (message->kind() != IrcModeMessage::User)
        return;
    sendCommand(IrcCommand::createNames(chan));
}

void UNO::onNames(IrcNamesMessage *message)
{
    foreach (QString w, message->names())
    {
        if (colors->allKeys().contains(startsWithMode(w) ? w.mid(1) : w))
            users->add(new User(startsWithMode(w) ? w.mid(1) : w, colors->value(startsWithMode(w) ? w.mid(1) : w).toInt(), startsWithMode(w) ? w.at(0) : QString()));
        else
            users->add(new User(startsWithMode(w) ? w.mid(1) : w, startsWithMode(w) ? w.at(0) : QString()));
    }
}

void UNO::onNick(IrcNickMessage *message)
{
    sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + message->newNick()));
    users->add(new User(message->newNick(), users->get(message->oldNick())->getColor(), users->get(message->oldNick())->getMode(), users->get(message->oldNick())->getColored()));
    users->remove(message->oldNick());
    if ((inGame || preGame) && players->contains(message->oldNick()))
    {
        Player *player = players->get(message->oldNick());
        players->add(new Player(message->newNick(), player->getDeck(), player->canPlay(), player->getColor()));
        players->remove(player);
        turns.replace(turns.indexOf(message->oldNick()), message->newNick());
        if (currPlayer == message->oldNick())
            currPlayer = message->newNick();
    }
}

void UNO::onNotice(IrcNoticeMessage *message)
{
    if (message->nick() == "NickServ" && message->content().contains("You are now identified"))
        sendCommand(IrcCommand::createJoin(chan));
    else if (message->content().startsWith("VERSION"))
    {
        sendCommand(IrcCommand::createMessage(chan, "\x02" + message->nick() + "\x0F" + " is using:" + message->content().mid(message->content().indexOf(" "))));
        inVersion = false;
    }
    else if (message->nick() == currPing && inPing)
    {
        if (QTime::currentTime().msecsSinceStartOfDay() - pingTimeBegin < pingTime)
            pingTime = QTime::currentTime().msecsSinceStartOfDay() - pingTimeBegin;
        pingCount++;

        if (pingCount < 3)
        {
            pingTimeBegin = QTime::currentTime().msecsSinceStartOfDay();
            sendCommand(IrcCommand::createCtcpRequest(currPing, "PING " + QString::number(pingTimeBegin)));
            return;
        }
        else
        {
            int lg = (int)(pingTime / 100);
            QString red, green;
            for (int i = 1; i < (50 - lg); i++)
                green.append(" ");
            while (red.length() + green.length() < 49)
                red.append(" ");
            sendCommand(IrcCommand::createMessage(chan, "\x03""01,03" + green + "\x03""01,04" + red + "\x0F""\x02"" " + users->get(currPing)->getColoredName() + ": " + QString::number(pingTime) + "ms "));
            currPing = "";
            pingTime = 10000000;
            pingCount = 0;
            inPing = false;
        }
    }
}

void UNO::onPart(IrcPartMessage *message)
{
    remPlayer(message->nick());
    users->remove(message->nick());
}

void UNO::onQuit(IrcQuitMessage *message)
{
    remPlayer(message->nick());
    users->remove(message->nick());
}

void UNO::pingTimeout()
{
    if (!inPing)
        return;
    sendCommand(IrcCommand::createMessage(chan, "\x02" + currPing + "\x0F" + " timed out"));
    inPing = false;
}

void UNO::versionTimeout(QString nick)
{
    if (!inVersion)
        return;
    sendCommand(IrcCommand::createMessage(chan, "\x02" + nick + "\x0F" + " timed out"));
    inVersion = false;
}

void UNO::preGameTimeout()
{
    if (!preGame)
        return;

    sendMessage("Timeout reached, the game is canceled");
    flushMessages();
    clear();
}

void UNO::showCards(QString nick, QString to)
{
    if (nick.isEmpty())
        nick = currPlayer;
    if (to.isEmpty())
        to = nick;
    sendNotice(to, players->get(nick)->getDeck()->toString(users->get(nick)->getColored()));
}

QString UNO::nextPlayer() const
{
    if (turns.indexOf(currPlayer) + (inversed ? -1 : 1) > turns.size() -1 || turns.indexOf(currPlayer) + (inversed ? -1 : 1) < 0)
        return inversed ? turns.last() : turns.first();
    else
        return turns.at(turns.indexOf(currPlayer) + (inversed ? -1 : 1));
}

void UNO::remPlayer(QString nick)
{
    if (players->contains(nick))
    {
        sendMessage(players->get(nick)->getColoredName() + " left the game");
        players->remove(nick);
        turns.removeOne(nick);

        if (players->size() == 1 && inGame)
        {
            sendMessage(players->get(players->first())->getColoredName() + " won the game!");
            clear();
        }
        else if (nick == currPlayer && inGame)
        {
            currPlayer = nextPlayer();
            sendMessage(players->get(currPlayer)->getColoredName() + ", it's your turn");
        }
        else if (players->size() == 0 && preGame)
        {
            sendMessage("No player left, the game is canceled");
            clear();
        }

        flushMessages();
    }
}

void UNO::clear()
{
    players->clear();
    turns.clear();
    inGame = false;
    preGame = false;
    drawed = false;
    inversed = false;
    delete pick;
    pick = new Cards(this);
}

void UNO::sendNotice(QString target, QString message)
{
    notices.insertMulti(target, message);
}

void UNO::sendMessage(QString message, Card *card)
{
    QString ncmessage = "\x02""[UNO] " + message;
    message = "\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14 " + message.replace("\x02", "\x03""04,15").replace("\x0F", "\x03""00,14") + " ";
    messages.append(card != 0 ? message.replace("%c", card->toString().replace("\x02", "\x03""04,15").replace("\x0F", "\x03""00,14")) : message);
    if (card != 0)
    {
        ncmessage.replace("%c", card->toString(false));
        foreach (User *w, users->getList())
            if (!w->getColored())
                sendNotice(w->getNick(), ncmessage);
    }
}

void UNO::flushMessages()
{
    if (messages.isEmpty() && notices.isEmpty())
        return;

    socket()->waitForBytesWritten();

    foreach (QString w, messages)
        sendCommand(IrcCommand::createMessage(chan, w));
    foreach (QString w, notices.keys())
        sendCommand(IrcCommand::createNotice(w, notices.value(w)));

    socket()->flush();
    messages.clear();
    notices.clear();
}

void UNO::command(QString nick, QString cmd, QStringList args)
{
    bool end = false;

    if (isOp(nick))
    {
        if (cmd == "exit" && args.isEmpty())
            qApp->exit();
        else if (cmd == "exit")
            qApp->exit(args.first().toInt());
        #ifndef Q_OS_WIN
        else if (cmd == "update")
        {
            sendMessage("Updating " + nickName());
            if (QProcess::startDetached(qApp->applicationDirPath() + "/updateUNO"))
                qApp->exit();
            else
                sendMessage("Unable to start updateUNO");
        }
        #endif
        else if (cmd == "kick" && !args.isEmpty())
            remPlayer(args.first());
        else if (cmd == "ban" && !args.isEmpty())
        {
            if (users->contains(args.first()))
            {
                bans->setValue(args.first(), users->get(args.first())->getHostname());
                sendMessage(users->get(args.first())->getColoredName() + " is now banned");
            }
            else
                sendMessage("\x02" + args.first() + "\x0F"" was not found");
        }
        else if (cmd == "unban" && !args.isEmpty())
        {
            if (users->contains(args.first()))
            {
                bans->remove(args.first());
                sendMessage(users->get(args.first())->getColoredName() + " is no longer banned");
            }
            else
                sendMessage("\x02" + args.first() + "\x0F"" was not found");
        }
        else if (cmd == "al" && accesslist->contains(nick) && !args.isEmpty())
        {
            if (args.first() == "add" && args.size() == 2)
            {
                if (accesslist->contains(args.at(1)))
                    sendMessage((users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F") + " is already in the access list");
                else if (users->contains(args.at(1)) && !users->get(args.at(1))->getHostname().isEmpty())
                {
                    accesslist->setValue(args.at(1), users->get(args.at(1))->getHostname());
                    sendMessage(users->get(args.at(1))->getColoredName() + " has been added");
                }
                else if (users->contains(args.at(1)))
                {
                    sendMessage(users->get(args.at(1))->getColoredName() + " must send a message in the channel before being added");
                }
                else
                    sendMessage("\x02" + args.at(1) + "\x0F"" was not found");
            }
            else if (args.first() == "del" && args.size() == 2)
            {
                if (accesslist->contains(args.at(1)))
                {
                    accesslist->remove(args.at(1));
                    sendMessage((users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F") + " has been removed");
                }
                else
                    sendMessage((users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F") + " is not in the access list");
            }
        }
    }

    if (cmd == "color") {
        if (args.isEmpty())
        {
            sendMessage("Usage : !color number");
            sendMessage("Available numbers : ""\x03""022  ""\x03""033  ""\x03""044  ""\x03""055  ""\x03""066  ""\x03""077  ""\x03""088  ""\x03""099  ""\x03""1010  ""\x03""1111  ""\x03""1212  ""\x03""1313");
            sendMessage("Current color : " + users->get(nick)->getColoredName());
        }
        else if (args.first().toInt() >= 2 && args.first().toInt() <= 13 && args.size() == 1)
        {
            if (players->contains(nick) && (inGame || preGame))
                sendMessage("You can't change your color during a game, " + players->get(nick)->getColoredName());
            else
            {
                users->get(nick)->setColor(args.first().toShort());
                colors->setValue(nick, args.first().toShort());
                sendMessage("You changed your color, " + users->get(nick)->getColoredName());
            }
        }
        else if (args.size() == 1)
            sendMessage(args.first() + " is not a valid color");
        else
            sendMessage("Usage : !color number");
    }
    else if (cmd == "nocolor") {
        users->get(nick)->setColored(!users->get(nick)->getColored());
        if (users->get(nick)->getColored())
            sendNotice(nick, "Color compatibility enabled");
        else
            sendNotice(nick, "Color compatibility disabled");
    }
    else if (cmd == "scores")
    {
        showScores();
    }
    else if (cmd == "help")
    {
        if (args.isEmpty())
        {
            sendMessage("!version, !rules, !color, !nocolor, !scores, !list, !quit, !uno, !join, !begin, !p, !draw, !end, !hand, !cards");
            sendMessage("Type ""\x02""!help <command>""\x0F"" to learn more about a command (example: !help join)");
        }
        else if (args.first() == "uno")
            sendMessage("\x02""Help :""\x0F"" !uno : commencer la préparation d'une nouvelle partie");
        else if (args.first() == "p")
        {
            sendMessage("\x02""Help :""\x0F"" !j");
            sendMessage("Utilisation : !j couleur carte");
            sendMessage("Exemple : pour jouer un %c il faut écrire !j v 7", new Card("V", "7"));
        }
        else if (args.first() == "join")
            sendMessage("\x02""Help :""\x0F"" !join : join the current game");
        else if (args.first() == "quit")
            sendMessage("\x02""Help :""\x0F"" !quit : leave the game");
        else if (args.first() == "begin")
            sendMessage("\x02""Help :""\x0F"" !begin : begin the game");
        else if (args.first() == "list")
            sendMessage("\x02""Help :""\x0F"" !list : display players list");
        else if (args.first() == "draw")
            sendMessage("\x02""Help :""\x0F"" !draw : draw a card");
        else if (args.first() == "end")
            sendMessage("\x02""Help :""\x0F"" !end : skip your turn");
        else if (args.first() == "hand")
            sendMessage("\x02""Help :""\x0F"" !hand : display your cards and the last played card");
        else if (args.first() == "cards")
            sendMessage("\x02""Help :""\x0F"" !cards : display the remaining cards count and the players cards count");
        else if (args.first() == "rules")
            sendMessage("\x02""Help :""\x0F"" !rules : send a link to the rules");
        else if (args.first() == "ping")
            sendMessage("\x02""Help :""\x0F"" !ping : display the latency between ""\x02" + nickName() + "\x0F"" and you");
        else if (args.first() == "version")
            sendMessage("\x02""Help :""\x0F"" !version : display the client version of the given user");
        else if (args.first() == "slaps")
            command(nickName(), "slaps", QStringList() << nick);
        else if (args.first() == "color")
            sendMessage("\x02""Help :""\x0F"" !color : choose your color");
        else if (args.first() == "nocolor")
            sendMessage("\x02""Help :""\x0F"" !nocolor : enable/disable the color compatibilty mode");
        else if (args.first() == "scores")
            sendMessage("\x02""Help :""\x0F"" !scores : display scores");
        else
            sendMessage("This command does not exist, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "rules")
        sendMessage("\x02""Rules (french) :""\x0F"" http://tuxange.org/unorules/");
    else if (cmd == "version")
    {
        if (args.isEmpty())
            args.append(nick);

        if (users->contains(args.first()))
        {
            sendCommand(IrcCommand::createVersion(args.first()));
            QTimer::singleShot(20000, this, SLOT(versionTimeout(nick)));
            inVersion = true;
        }
        else
            sendCommand(IrcCommand::createMessage(chan, "\x02" + args.first() + "\x0F"" was not found"));
    }
    else if (cmd == "slaps")
    {
        if (args.isEmpty())
            args.append(nickName());
        sendCommand(IrcCommand::createMessage(chan, "\x02""\x03""00,14" + users->get(nick)->getColoredName() + " " + slaps->value(QString::number(qrand() % slaps->allKeys().size())).toString().replace("%s", (users->contains(args.first()) ? users->get(args.first())->getColoredName() : "\x03""04,15" + args.first() + "\x03""00,14"))));
    }
    else if (cmd == "ping")
    {
        if (!inPing)
        {
            pingTime = 1000000;
            pingCount = 0;
            inPing = true;
            currPing = nick;
            pingTimeBegin = QTime::currentTime().msecsSinceStartOfDay();
            sendCommand(IrcCommand::createCtcpRequest(nick, "PING " + QString::number(pingTimeBegin)));
            QTimer::singleShot(20000, this, SLOT(pingTimeout()));
        }
        else
            sendCommand(IrcCommand::createMessage(chan, "A ping is already in process, " + (players->contains(nick) ? players->get(nick)->getColoredName() : "\x02" + nick)));
    }
    else if (cmd == "list")
    {
        if (inGame || preGame)
            sendMessage("In the game: " + inGame ? showTurns() : players->list());
        else
            sendMessage("There is no game, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "quit")
    {
        if (players->contains(nick))
            remPlayer(nick);
        else if (!inGame)
            sendMessage("There is no game, " + users->get(nick)->getColoredName());
        else
            sendMessage("You are not in this game, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "join")
    {
        if (preGame)
        {
            if (!players->contains(nick))
            {
                Player *p = new Player(nick, this);
                players->add(p);
                turns.insert(qrand() % turns.size(), nick);
                sendMessage(p->getColoredName() + " joined this game");
                sendMessage("There are " + QString::number(players->size()) + " players in game");
            }
            else
                sendMessage("You are already in this game, " + players->get(nick)->getColoredName());
        }
        else if (inGame)
            sendMessage("You can't join a launched game, " + users->get(nick)->getColoredName());
        else
            sendMessage("There is no game, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "begin")
    {
        if (!players->contains(nick) && preGame)
            sendMessage("You are not in this game, " + users->get(nick)->getColoredName());
        else if (players->size() > 1 && preGame)
        {
            pick->randomize();
            do { lastCard = pick->pick(); } while (lastCard->getId() == "+4");
            foreach (Player *w, players->getList())
                w->getDeck()->init();
            sendMessage(" --- ");
            sendMessage("Last card: %c", lastCard);
            currPlayer = players->rand()->getName();

            if (lastCard->getId() == "+2")
            {
                sendMessage(players->get(currPlayer)->getColoredName() + " draws 2 cards");
                sendNotice(currPlayer, players->get(currPlayer)->getDeck()->randCards(2, users->get(currPlayer)->getColored()));
                sendMessage(players->get(currPlayer)->getColoredName() + " skips his turn");
                currPlayer = nextPlayer();
            }
            else if (lastCard->getId() == "I" && players->size() > 2)
            {
                sendMessage("Turns are ""\x16""reversed!");
                inversed = true;
            }
            else if (lastCard->getId() == "P" || lastCard->getId() == "I")
            {
                sendMessage(players->get(currPlayer)->getColoredName() + " skips his turn");
                currPlayer = nextPlayer();
            }

            sendMessage(players->get(currPlayer)->getColoredName() + ", it's your turn");

            foreach (Player *w, players->getList())
                showCards(w->getName());

            inGame = true;
            preGame = false;
        }
        else if (preGame)
            sendMessage("There is not enough players to launch the game, " + players->get(nick)->getColoredName());
        else if (inGame)
            sendMessage("A game is already launched, " + users->get(nick)->getColoredName());
        else
            sendMessage("There is no game, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "uno")
    {
        if (!inGame && !preGame)
        {
            preGame = true;
            Player *p = new Player(nick, this);
            players->add(p);
            turns.append(nick);
            sendMessage(p->getColoredName() + " created a new game");
            sendMessage(p->getColoredName() + " joined the game");
            sendMessage("There is 1 player in this game");
            QTimer::singleShot(60*5*1000, this, SLOT(preGameTimeout()));
        }
        else if (inGame)
            sendMessage("A game is already launched, " + users->get(nick)->getColoredName());
        else // preGame
            sendMessage("A game is already being created, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "draw" || cmd == "d")
    {
        if (!inGame)
            sendMessage("There is no game, " + users->get(nick)->getColoredName());
        else if (!drawed && currPlayer == nick)
        {
            sendNotice(currPlayer, players->get(currPlayer)->getDeck()->randCards(1));
            sendMessage("Last card: %c", lastCard);
            drawed = true;
        }
        else if (currPlayer == nick)
            sendMessage("You already drew, " + players->get(nick)->getColoredName());
        else
            sendMessage(players->get(currPlayer)->getColoredName() + ", it's your turn");
    }
    else if (cmd == "end" || cmd == "e")
    {
        if (!inGame)
            sendMessage("There is no game, " + users->get(nick)->getColoredName());
        else if (drawed && currPlayer == nick)
            end = true;
        else if (currPlayer == nick)
            sendMessage("You did not draw, " + players->get(currPlayer)->getColoredName());
        else
            sendMessage(players->get(currPlayer)->getColoredName() + ", it's your turn");
    }
    else if (cmd == "hand")
    {
        if (!inGame)
            sendMessage("There is no game, " + users->get(nick)->getColoredName());
        else if (!isOp(nick) && !args.isEmpty())
            sendMessage("You can't see the other players cards, " + users->get(nick)->getColoredName());
        else if (args.isEmpty())
        {
            sendMessage("Last card: %c", lastCard);
            sendMessage(players->get(currPlayer)->getColoredName() + ", it's your turn");
            if (players->contains(nick)) showCards(nick);
        }
        else if (isOp(nick))
        {
            if (players->contains(args.at(0)))
                showCards(args.at(0), nick);
            else
                sendMessage("\x02" + args.at(0) + "\x0F"" was not found, " + users->get(nick)->getColoredName());
            flushMessages();
        }
    }
    else if (cmd == "cards")
    {
        if (!inGame)
            sendMessage("There is no game, " + users->get(nick)->getColoredName());
        else
        {
            sendMessage(QString::number(pick->size()) + " cards remaining");
            foreach (Player *w, players->getList())
                sendMessage(w->getColoredName() + " has " + QString::number(w->getDeck()->size()) + " cards");
        }
    }
    else if (cmd == "p")
    {
        Player *curr = players->get(currPlayer);
        if (!inGame)
            sendMessage("There is no game, " + users->get(nick)->getColoredName());
        else if (currPlayer != nick)
            sendMessage(curr->getColoredName() + ", it's your turn");
        else if (args.size() < 2)
            sendMessage("\x02""!help""\x0F"" p to learn how to use it");
        else if (curr->getDeck()->contains(new Card(args.at(0), args.at(1))) || curr->getDeck()->contains(new Card("N", args.at(1))))
        {
            QString color = QString(args.at(0)).toUpper(), id = QString(args.at(1)).toUpper();
            if (lastCard->getColor() == color || lastCard->getId() == id || lastCard->getColor() == "N" || id == "J" || id == "+4")
            {
                if (color == "R" || color == "G" || color == "B" || color == "Y")
                {
                    if (id == "J")
                    {
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard("N", id);
                        end = true;
                    }
                    else if (id == "+4")
                    {
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard("N", id);
                        Player *next = players->get(nextPlayer());
                        sendMessage(next->getColoredName() + " draws 4 cards");
                        sendNotice(next->getName(), next->getDeck()->randCards(4, users->get(next->getName())->getColored()));
                        next->cantPlay();
                        end = true;
                    }
                    else if (id == "+2")
                    {
                        Player *next = players->get(nextPlayer());
                        sendMessage(next->getColoredName() + " draws 2 cards");
                        sendNotice(next->getName(), next->getDeck()->randCards(2, users->get(next->getName())->getColored()));
                        next->cantPlay();
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard(color, id);
                        end = true;
                    }
                    else if (id == "I")
                    {
                        if (players->size() == 2)
                            players->get(nextPlayer())->cantPlay();
                        else
                        {
                            sendMessage("Turns are ""\x16""reversed!");
                            inversed = !inversed;
                        }
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard(color, id);
                        end = true;
                    }
                    else if (id == "P")
                    {
                        players->get(nextPlayer())->cantPlay();
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard(color, id);
                        end = true;
                    }
                    else
                    {
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard(color, id);
                        end = true;
                    }
                }
                else
                    sendMessage("This color does not exist, " + curr->getColoredName());
            }
            else
                sendMessage("You can't play this card, " + curr->getColoredName());
        }
        else
        {
            bool ok;
            QString color = QString(args.at(0)).toUpper(), id = QString(args.at(1)).toUpper();
            if (((id == "+2" || id == "I" || id == "P" || (id.toInt(&ok, 10) < 10 && ok)) || (id == "+4" || id == "J")) && (color == "R" || color == "G" || color == "B" || color == "Y"))
                sendMessage("You don't have this card, " + curr->getColoredName());
            else
                sendMessage("This card does not exist, " + curr->getColoredName());
        }
    }

    if (end && players->get(currPlayer)->getDeck()->size() == 1)
        sendMessage(players->get(currPlayer)->getColoredName() + " ""\x16""is ""\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14""\x16"" !");
    else if (end && players->get(currPlayer)->getDeck()->size() == 0)
    {
        sendMessage(players->get(currPlayer)->getColoredName() + " won the game !");

        scores->setValue(currPlayer, scores->value(currPlayer, 0).toInt() + 1);
        int points = scores->value("Points/" + currPlayer, 0).toInt(), origpoints = points;

        foreach (Player *w, players->getList())
        {
            scores->setValue("Total/" + w->getName(), scores->value("Total/" + w->getName(), 0).toInt() + 1);
            if (w->getName() != currPlayer)
                foreach (Card *c, w->getDeck()->getList())
                {
                    if (c->getId() == "+4" || c->getId() == "J")
                        points += 50;
                    else if (c->getId() == "I" || c->getId() == "P" || c->getId() == "+2")
                        points += 20;
                    else
                        points += c->getId().toInt();
                }
        }

        scores->setValue("Points/" + currPlayer, points);
        sendMessage(players->get(currPlayer)->getColoredName() + " earned " + QString::number(points - origpoints) + " points !");

        sendMessage(" --- ");
        sendMessage("Type ""\x02""!scores""\x0F"" to show the scores");
        flushMessages();
        clear();
        return;
    }

    if (end)
    {
        currPlayer = nextPlayer();
        drawed = false;
        if (!players->get(currPlayer)->canPlay())
        {
            sendMessage(players->get(currPlayer)->getColoredName() + " skips his turn");
            sendMessage(" --- ");
            sendMessage("Last Card: %c", lastCard);
            currPlayer = nextPlayer();
            sendMessage(players->get(currPlayer)->getColoredName() + ", it's your turn");
            showCards();
        }
        else
        {
            sendMessage(" --- ");
            sendMessage("Last card: %c", lastCard);
            sendMessage(players->get(currPlayer)->getColoredName() + ", it's your turn");
            showCards();
        }
    }

    flushMessages();
}

QString UNO::showTurns() const
{
    QString ret;
    foreach (QString w, turns)
        ret += players->get(w)->getColoredName() + ", ";
    ret.chop(2);
    return ret;
}

void UNO::showScores()
{
    if (scores->allKeys().isEmpty())
    {
        sendMessage("Scores are still empty");
        return;
    }

    sendMessage("Scores:");
    QStringList people = scores->allKeys();

    foreach (QString w, people)
        if (w.startsWith("Total/") || w.startsWith("Points/"))
            people.removeOne(w);

    QString curr;
    double ratio = -1;
    double currratio;

    for (int i = 0; i < 10; i++)
    {
        curr = "";
        ratio = -1;
        foreach (QString w, people)
        {
            currratio = ((scores->value("Points/" + w).toInt()/6) + (scores->value(w).toInt()*105/scores->value("Total/" + w).toInt()) + (scores->value(w).toInt()*8)) / 4;
            if (currratio > ratio)
            {
                curr = w;
                ratio = currratio;
            }
        }
        sendMessage((i == 9 ? "" : " ") + QString::number(i + 1) + ". " + (users->contains(curr) ? users->get(curr)->getColoredName().insert(7, "\u200B") : "\x02" + curr + "\x0F") + " : " + QString::number((int)ratio) + " (" + scores->value("Points/" + curr).toString() + " points for " + scores->value(curr).toString() + " win" + (scores->value(curr).toInt() > 1 ? "s" : "") + " on " + scores->value("Total/" + curr).toString() + " game" + (scores->value("Total/" + curr).toInt() > 1 ? "s" : "") + " played)");
        people.removeOne(curr);

        if (people.isEmpty())
            break;
    }

    flushMessages();
}

bool UNO::isOp(QString user)
{
    if (accesslist->contains(user))
    {
        if (getUsers()->get(user)->getHostname() == accesslist->value(user))
            return true;
        else
            return false;
    }
    else
        return network()->prefixToMode(users->get(user)->getMode()) == "o" || network()->prefixToMode(users->get(user)->getMode()) == "q" ? true : false;
}

bool UNO::startsWithMode(QString nick)
{
    QString w = nick.at(0);
    if (w == network()->modeToPrefix("q") || w == network()->modeToPrefix("a") || w == network()->modeToPrefix("o") || w == network()->modeToPrefix("h") || w == network()->modeToPrefix("v"))
        return true;
    return false;
}

Cards* UNO::getCards() const
{
    return pick;
}

Users* UNO::getUsers() const
{
    return users;
}

Players* UNO::getPlayers() const
{
    return players;
}
