#include "uno.h"

UNO::UNO(QCoreApplication *_parent) : IrcConnection(_parent)
{
    qsrand(QTime::currentTime().msec());

    verbose = 0;
    pick = new Cards(this);
    players = new Players(this);
    users = new Users;
    lastCard = new Card("", "");
    inGame = false, preGame = false, drawed = false, inversed = false, inPing = false, inVersion = false;
    updater = new Updater(qApp->applicationDirPath(), this);

    log(INIT, tr("Loading ini files..."));

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

    verbose = settings->value("verbose", 0).toInt();
    log(INIT, tr("Verbose level set to %1").arg(QString::number(verbose)));
    if (verbose >= 3)
        qputenv("IRC_DEBUG", "1");

    if (settings->allKeys().isEmpty())
        log(WARNING, tr("settings.ini is empty or missing, using default values"));

    log(INIT, tr("UNO [Update %1] initialised").arg(COMMITDATE));
    setHost(settings->value("server", "irc.freenode.net").toString());
    setPort(settings->value("port", 6667).toInt());
    setSecure(settings->value("ssl", false).toBool());
    chan = (settings->value("chan", "##newuno").toString().startsWith("#") ? "" : "#") + settings->value("chan", "##newuno").toString();
    setEncoding(settings->value("encoding", "UTF-8").toByteArray());
    setUserName(settings->value("username", "UNO" + QString::number(qrand())).toString());
    setNickName(settings->value("nickname", userName()).toString());
    setRealName(settings->value("realname", userName()).toString());
    setReconnectDelay(5);

    log(INFO, tr("Server: %1 on port %2").arg(host()).arg(QString::number(port())));
    log(INFO, tr("SSL: %1").arg(isSecure() ? tr("true") : tr("false")));
    log(INFO, tr("Channel: %1").arg(chan));
    log(INFO, tr("Encoding: %1").arg(QString(encoding())));
    log(INFO, tr("Username: %1").arg(userName()));
    log(INFO, tr("Nickname: %1").arg(nickName()));
    log(INFO, tr("Realname: %1").arg(realName()));

    QVariantMap CtcpReplies;
    CtcpReplies.insert("VERSION", "\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14""\x16"" " + tr("[Update %1]").arg(COMMITDATE));
    CtcpReplies.insert("SOURCE", "https://github.com/TheShayy/UNO");
    setCtcpReplies(CtcpReplies);

    log(INFO, tr("Connecting"));
    open();

    QObject::connect(this, SIGNAL(secureError()), this, SLOT(onSSLError()));
    QObject::connect(this, SIGNAL(connected()), this, SLOT(onConnect()));
    QObject::connect(this, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    QObject::connect(this, SIGNAL(messageReceived(IrcMessage*)), this, SLOT(onIrcMessage(IrcMessage*)));
    QObject::connect(this, SIGNAL(privateMessageReceived(IrcPrivateMessage*)), this, SLOT(onMessage(IrcPrivateMessage*)));
    QObject::connect(this, SIGNAL(joinMessageReceived(IrcJoinMessage*)), this, SLOT(onJoin(IrcJoinMessage*)));
    QObject::connect(this, SIGNAL(kickMessageReceived(IrcKickMessage*)), this, SLOT(onKick(IrcKickMessage*)));
    QObject::connect(this, SIGNAL(modeMessageReceived(IrcModeMessage*)), this, SLOT(onMode(IrcModeMessage*)));
    QObject::connect(this, SIGNAL(namesMessageReceived(IrcNamesMessage*)), this, SLOT(onNames(IrcNamesMessage*)));
    QObject::connect(this, SIGNAL(nickMessageReceived(IrcNickMessage*)), this, SLOT(onNick(IrcNickMessage*)));
    QObject::connect(this, SIGNAL(noticeMessageReceived(IrcNoticeMessage*)), this, SLOT(onNotice(IrcNoticeMessage*)));
    QObject::connect(this, SIGNAL(partMessageReceived(IrcPartMessage*)), this, SLOT(onPart(IrcPartMessage*)));
    QObject::connect(this, SIGNAL(quitMessageReceived(IrcQuitMessage*)), this, SLOT(onQuit(IrcQuitMessage*)));

    QObject::connect(updater, SIGNAL(step(QString)), this, SLOT(onUpdaterStep(QString)));
    QObject::connect(updater, SIGNAL(error(QString)), this, SLOT(onUpdaterError(QString)));
    QObject::connect(updater, SIGNAL(done()), this, SLOT(onUpdaterDone()));
}

UNO::~UNO()
{
    log(INFO, tr("Closing UNO"));
    if (isActive())
    {
        quit(tr("Closed"));
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

void UNO::onSSLError()
{
    log(ERROR, tr("SSL error, set verbose level to 3 to show the details"));
}

void UNO::onConnect()
{
    if (!settings->value("nspassword", QString()).toString().isEmpty())
    {
        log(INFO, "Connected, logging in to NickServ");
        sendCommand(IrcCommand::createMessage("NickServ", "IDENTIFY " + settings->value("nspassword").toString()));
    }
    else
    {
        log(INFO, tr("Connected, joining %1").arg(chan));
        sendCommand(IrcCommand::createJoin(chan));
    }
}

void UNO::onDisconnect()
{
    log(WARNING, tr("Disconnected"));
}

void UNO::onIrcMessage(IrcMessage *message)
{
    bool ok;
    int code = message->command().toInt(&ok);
    if (!ok)
        return;

    switch (code)
    {
        case 431:
            log(ERROR, tr("%1 said nickname is empty, exiting").arg(host()));
            qApp->exit(1);
        break;
        case 432:
            log(ERROR, tr("Erroneous nickname, exiting"));
            qApp->exit(1);
        break;
        case 433:
            log(ERROR, tr("%1 is already in use, exiting").arg(nickName()));
            qApp->exit(1);
        break;
    }
}

void UNO::onMessage(IrcPrivateMessage *message)
{
    if (users->get(message->nick())->getHostname().isEmpty())
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
        log(INFO, tr("%1%2 issued command %3").arg(users->get(message->nick())->getMode() + message->nick()).arg(isOp(message->nick()) ? " (with admin rights)" : "").arg(message->content()));
        QStringList args = message->content().split(" ", QString::SkipEmptyParts);
        args.removeFirst();
        command(message->nick(), message->content().split(" ", QString::SkipEmptyParts).first().mid(1), args);
    }
}

void UNO::onJoin(IrcJoinMessage *message)
{
    sendCommand(IrcCommand::createNames(chan));
    users->get(message->nick())->setHostname(message->host());
    sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + message->nick()));
    if (preGame)
        sendMessage(tr("Wanna play? Try the %1 command!").arg("\x02""!join""\x0F"));
    else if (!inGame)
        sendMessage(tr("Wanna play? Try the %1 command!").arg("\x02""!help""\x0F"));
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
    log(INFO, tr("%1 is now %2").arg(message->oldNick()).arg(message->nick()));
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
    {
        log(INFO, tr("Identified. Joining %1").arg(chan));
        sendCommand(IrcCommand::createJoin(chan));
    }
    else if (message->content().startsWith("VERSION"))
    {
        sendCommand(IrcCommand::createMessage(chan, tr("%1 is using: %2").arg("\x02" + message->nick() + "\x0F").arg(message->content().mid(message->content().indexOf(" ")))));
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

void UNO::onUpdaterStep(QString step) {
    if (step == "git")
        sendMessage(tr("Running git"), NULL, true);
    else if (step == "configure")
        sendMessage(tr("Running configure"), NULL, true);
    else if (step == "make")
        sendMessage(tr("Building"), NULL, true);
    else if (step == "files")
        sendMessage(tr("Replacing old files"), NULL, true);
}

void UNO::onUpdaterError(QString step) {
    sendMessage(tr("Error processing step '%1'").arg(step), NULL, true);
}

void UNO::onUpdaterDone() {
    sendMessage(tr("Launching freshly made UNObot"), NULL, true);
    quit(tr("Updating..."));
    QProcess::startDetached("/bin/bash", QStringList() << qApp->applicationDirPath() + "/startUNO", qApp->applicationDirPath());
    qApp->exit();
}

void UNO::pingTimeout()
{
    if (!inPing)
        return;
    sendCommand(IrcCommand::createMessage(chan, tr("%1 timed out").arg(users->contains(currPing) ? users->get(currPing)->getColoredName() : "\x02" + currPing + "\x0F")));
    inPing = false;
}

void UNO::versionTimeout()
{
    if (!inVersion)
        return;
    sendCommand(IrcCommand::createMessage(chan, tr("%1 timed out").arg(users->contains(currVersion) ? users->get(currVersion)->getColoredName() : "\x02" + currVersion + "\x0F")));
    inVersion = false;
}

void UNO::preGameTimeout()
{
    if (!preGame)
        return;

    log(INFO, tr("Timeout, the game is canceled"));
    sendMessage(tr("Timeout, the game is canceled"));
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
    if (!players->contains(nick))
        return;

    log(INFO, tr("%1 left the game").arg(nick));
    sendMessage(tr("%1 left the game").arg(players->get(nick)->getColoredName()));
    players->remove(nick);
    turns.removeOne(nick);

    if (players->size() == 1 && inGame)
    {
        log(INFO, tr("%1 won the game").arg(players->first()));
        sendMessage(tr("%1 won the game!").arg(players->get(players->first())->getColoredName()));
        clear();
    }
    else if (nick == currPlayer && inGame)
    {
        currPlayer = nextPlayer();
        sendMessage(players->get(currPlayer)->getColoredName() + ", it's your turn");
    }
    else if (players->size() == 0 && preGame)
    {
        log(INFO, tr("No player left, the game is canceled"));
        sendMessage(tr("No player left, the game is canceled"));
        clear();
    }

    flushMessages();
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

void UNO::sendMessage(QString message, Card *card, bool direct)
{
    QString ncmessage = "\x02""[UNO] " + message;
    message = "\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14 " + message.replace("\x02", "\x03""04,15").replace("\x0F", "\x03""00,14") + " ";

    if (!direct)
        messages.append(card != NULL ? message.replace("%c", card->toString().replace("\x02", "\x03""04,15").replace("\x0F", "\x03""00,14")) : message);
    else
        sendCommand(IrcCommand::createMessage(chan, card != NULL ? message.replace("%c", card->toString().replace("\x02", "\x03""04,15").replace("\x0F", "\x03""00,14")) : message));

    if (card != NULL)
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
            sendMessage(tr("Updating %1").arg(nickName()), NULL, true);
            updater->start();
        }
        #endif
        else if (cmd == "kick" && !args.isEmpty())
            remPlayer(args.first());
        else if (cmd == "ban" && !args.isEmpty())
        {
            if (users->contains(args.first()))
            {
                bans->setValue(args.first(), users->get(args.first())->getHostname());
                sendMessage(tr("%1 is now banned").arg(users->get(args.first())->getColoredName()));
            }
            else
                sendMessage(tr("%1 was not found").arg("\x02" + args.first() + "\x0F"));
        }
        else if (cmd == "unban" && !args.isEmpty())
        {
            if (users->contains(args.first()))
            {
                bans->remove(args.first());
                sendMessage(tr("%1 is no longer banned").arg(users->get(args.first())->getColoredName()));
            }
            else
                sendMessage(tr("%1 was not found").arg("\x02" + args.first() + "\x0F"));
        }
        else if (cmd == "al" && accesslist->contains(nick) && !args.isEmpty())
        {
            if (args.first() == "add" && args.size() == 2)
            {
                if (accesslist->contains(args.at(1)))
                    sendMessage(tr("%1 is already in the access list").arg(users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F"));
                else if (users->contains(args.at(1)))
                {
                    accesslist->setValue(args.at(1), users->get(args.at(1))->getHostname());
                    sendMessage(tr("%1 has been added").arg(users->get(args.at(1))->getColoredName()));
                }
                else
                    sendMessage(tr("%1 was not found").arg("\x02" + args.at(1) + "\x0F"));
            }
            else if (args.first() == "del" && args.size() == 2)
            {
                if (accesslist->contains(args.at(1)))
                {
                    accesslist->remove(args.at(1));
                    sendMessage(tr("%1 has been removed").arg(users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F"));
                }
                else
                    sendMessage(tr("%1 is not in the access list").arg(users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F"));
            }
            else if (args.first() == "list")
            {
                sendMessage(tr("In the access list: %1").arg(accesslist->allKeys().join(", ")));
            }
        }
    }

    if (cmd == "color") {
        if (args.isEmpty())
        {
            sendMessage(tr("Usage : !color number"));
            sendMessage(tr("Available numbers : %1").arg("\x03""022  ""\x03""033  ""\x03""044  ""\x03""055  ""\x03""066  ""\x03""077  ""\x03""088  ""\x03""099  ""\x03""1010  ""\x03""1111  ""\x03""1212  ""\x03""1313"));
            sendMessage(tr("Current color : %1").arg(users->get(nick)->getColoredName()));
        }
        else if (args.first().toInt() >= 2 && args.first().toInt() <= 13 && args.size() == 1)
        {
            if (players->contains(nick) && (inGame || preGame))
                sendMessage(tr("You can't change your color during a game, %1").arg(players->get(nick)->getColoredName()));
            else
            {
                users->get(nick)->setColor(args.first().toShort());
                colors->setValue(nick, args.first().toShort());
                sendMessage(tr("You changed your color, %1").arg(users->get(nick)->getColoredName()));
            }
        }
        else if (args.size() == 1)
            sendMessage(tr("%1 is not a valid color").arg(args.first()));
        else
            sendMessage(tr("Usage : !color number"));
    }
    else if (cmd == "nocolor") {
        users->get(nick)->setColored(!users->get(nick)->getColored());
        if (users->get(nick)->getColored())
            sendNotice(nick, tr("Color compatibility enabled"));
        else
            sendNotice(nick, tr("Color compatibility disabled"));
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
            sendMessage(tr("Type %1 to learn more about a command (example: !help join)").arg("\x02""!help <command>""\x0F"));
            sendMessage(tr("Colors: %1 %2 %3 %4").arg("\x02""\x03""01,04""[r] " + tr("red") + "\x0F").arg("\x02""\x03""01,03""[g] " + tr("green") + "\x0F").arg("\x02""\x03""01,11""[b] " + tr("blue") + "\x0F").arg("\x02""\x03""01,08""[y] " + tr("yellow") + "\x0F"));
        }
        else if (args.first() == "uno")
            sendMessage(tr("%1 !uno : create a new game").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "p")
        {
            sendMessage(tr("%1 !p").arg("\x02" + tr("Help :") + "\x0F"));
            sendMessage(tr("Usage : !p color card"));
            sendMessage(tr("Example : to play a %c you should type !p g 7"), new Card("G", "7"));
        }
        else if (args.first() == "join")
            sendMessage(tr("%1 !join : join the current game").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "quit")
            sendMessage(tr("%1 !quit : leave the game").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "begin")
            sendMessage(tr("%1 !begin : begin the game").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "list")
            sendMessage(tr("%1 !list : display players list").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "draw")
            sendMessage(tr("%1 !draw : draw a card").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "end")
            sendMessage(tr("%1 !end : skip your turn").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "hand")
            sendMessage(tr("%1 !hand : display your cards and the last played card").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "cards")
            sendMessage(tr("%1 !cards : display the remaining cards count and the players cards count").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "rules")
            sendMessage(tr("%1 !rules : send a link to the rules").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "ping")
            sendMessage(tr("%1 !ping : display the latency between %2 and you").arg("\x02" + tr("Help :") + "\x0F").arg("\x02" + nickName() + "\x0F"));
        else if (args.first() == "version")
            sendMessage(tr("%1 !version : display the client version of the given user").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "slaps")
            command(nickName(), "slaps", QStringList() << nick);
        else if (args.first() == "color")
            sendMessage(tr("%1 !color : choose your color").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "nocolor")
            sendMessage(tr("%1 !nocolor : enable/disable the color compatibilty mode").arg("\x02" + tr("Help :") + "\x0F"));
        else if (args.first() == "scores")
            sendMessage(tr("%1 !scores : display scores").arg("\x02" + tr("Help :") + "\x0F"));
        else
            sendMessage(tr("This command does not exist, %1").arg(users->get(nick)->getColoredName()));
    }
    else if (cmd == "rules")
        sendMessage(tr("%1 http://tuxange.org/unorules/").arg("\x02" + tr("Rules (french) :") + "\x0F"));
    else if (cmd == "version")
    {
        if (inVersion)
        {
            sendCommand(IrcCommand::createMessage(chan, tr("Waiting for %1's answer").arg(users->get(currVersion)->getColoredName())));
            return;
        }

        if (args.isEmpty())
            args.append(nick);

        if (users->contains(args.first()))
        {
            sendCommand(IrcCommand::createVersion(args.first()));
            QTimer::singleShot(20000, this, SLOT(versionTimeout()));
            inVersion = true;
            currVersion = args.first();
        }
        else
            sendCommand(IrcCommand::createMessage(chan, tr("%1 was not found").arg("\x02" + args.first() + "\x0F")));
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
            sendCommand(IrcCommand::createMessage(chan, tr("A ping is already in process, %1").arg(players->contains(nick) ? players->get(nick)->getColoredName() : "\x02" + nick)));
    }
    else if (cmd == "list")
    {
        if (inGame || preGame)
            sendMessage(tr("In the game: %1").arg(inGame ? showTurns() : players->list()));
        else
            sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
    }
    else if (cmd == "quit")
    {
        if (players->contains(nick))
            remPlayer(nick);
        else if (!inGame)
            sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
        else
            sendMessage(tr("You are not in this game, %1").arg(users->get(nick)->getColoredName()));
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
                sendMessage(tr("%1 joined this game").arg(p->getColoredName()));
                sendMessage(tr("There are %1 players in game").arg(QString::number(players->size())));
            }
            else
                sendMessage(tr("You are already in this game, %1").arg(players->get(nick)->getColoredName()));
        }
        else if (inGame)
            sendMessage(tr("You can't join a launched game, %1").arg(users->get(nick)->getColoredName()));
        else
            sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
    }
    else if (cmd == "begin")
    {
        if (!players->contains(nick) && preGame)
            sendMessage(tr("You are not in this game, %1").arg(users->get(nick)->getColoredName()));
        else if (players->size() > 1 && preGame)
        {
            pick->randomize();
            do { lastCard = pick->pick(); } while (lastCard->getId() == "+4");
            foreach (Player *w, players->getList())
                w->getDeck()->init();
            sendMessage(" --- ");
            sendMessage(tr("Last card: %c"), lastCard);
            currPlayer = players->rand()->getName();

            if (lastCard->getId() == "+2")
            {
                sendMessage(tr("%1 draws 2 cards").arg(players->get(currPlayer)->getColoredName()));
                sendNotice(currPlayer, players->get(currPlayer)->getDeck()->randCards(2, users->get(currPlayer)->getColored()));
                sendMessage(tr("%1 skips his turn").arg(players->get(currPlayer)->getColoredName()));
                currPlayer = nextPlayer();
            }
            else if (lastCard->getId() == "I" && players->size() > 2)
            {
                sendMessage(tr("Turns are %1 reversed!").arg("\x16"));
                inversed = true;
            }
            else if (lastCard->getId() == "P" || lastCard->getId() == "I")
            {
                sendMessage(tr("%1 skips his turn").arg(players->get(currPlayer)->getColoredName()));
                currPlayer = nextPlayer();
            }

            sendMessage(tr("%1, it's your turn").arg(players->get(currPlayer)->getColoredName()));

            foreach (Player *w, players->getList())
                showCards(w->getName());

            inGame = true;
            preGame = false;
        }
        else if (preGame)
            sendMessage(tr("There is not enough players to launch the game, %1").arg(players->get(nick)->getColoredName()));
        else if (inGame)
            sendMessage(tr("A game is already launched, %1").arg(users->get(nick)->getColoredName()));
        else
            sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
    }
    else if (cmd == "uno")
    {
        if (!inGame && !preGame)
        {
            preGame = true;
            Player *p = new Player(nick, this);
            players->add(p);
            turns.append(nick);
            sendMessage(tr("%1 created a new game").arg(p->getColoredName()));
            sendMessage(tr("%1 joined the game").arg(p->getColoredName()));
            sendMessage(tr("There is 1 player in this game"));
            QTimer::singleShot(300000, this, SLOT(preGameTimeout()));
        }
        else if (inGame)
            sendMessage(tr("A game is already launched, %1").arg(users->get(nick)->getColoredName()));
        else // preGame
            sendMessage(tr("A game is already being created, %1").arg(users->get(nick)->getColoredName()));
    }
    else if (cmd == "draw" || cmd == "d")
    {
        if (!inGame)
            sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
        else if (!drawed && currPlayer == nick)
        {
            sendNotice(currPlayer, players->get(currPlayer)->getDeck()->randCards(1));
            sendMessage(tr("Last card: %c"), lastCard);
            drawed = true;
        }
        else if (currPlayer == nick)
            sendMessage(tr("You already drew, %1").arg(players->get(nick)->getColoredName()));
        else
            sendMessage(tr("%1, it's your turn").arg(players->get(currPlayer)->getColoredName()));
    }
    else if (cmd == "end" || cmd == "e")
    {
        if (!inGame)
            sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
        else if (drawed && currPlayer == nick)
            end = true;
        else if (currPlayer == nick)
            sendMessage(tr("You did not draw, %1").arg(players->get(currPlayer)->getColoredName()));
        else
            sendMessage(tr("%1, it's your turn").arg(players->get(currPlayer)->getColoredName()));
    }
    else if (cmd == "hand")
    {
        if (!inGame)
            sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
        else if (!isOp(nick) && !args.isEmpty())
            sendMessage(tr("You can't see the other players cards, %1").arg(users->get(nick)->getColoredName()));
        else if (args.isEmpty())
        {
            sendMessage(tr("Last card: %c"), lastCard);
            sendMessage(tr("%1, it's your turn").arg(players->get(currPlayer)->getColoredName()));
            if (players->contains(nick)) showCards(nick);
        }
        else if (isOp(nick))
        {
            if (players->contains(args.at(0)))
                showCards(args.at(0), nick);
            else
                sendMessage(tr("%1 was not found, %2").arg("\x02" + args.at(0) + "\x0F").arg(users->get(nick)->getColoredName()));
            flushMessages();
        }
    }
    else if (cmd == "cards")
    {
        if (!inGame)
            sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
        else
        {
            sendMessage(tr("%1 cards remaining").arg(QString::number(pick->size())));
            foreach (Player *w, players->getList())
                sendMessage(tr("%1 has %2 cards").arg(w->getColoredName()).arg(QString::number(w->getDeck()->size())));
        }
    }
    else if (cmd == "p")
    {
        Player *curr = players->get(currPlayer);
        if (!inGame)
            sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
        else if (currPlayer != nick)
            sendMessage(tr("%1, it's your turn").arg(curr->getColoredName()));
        else if (args.size() < 2)
            sendMessage(tr("%1 to learn how to use it").arg("\x02""!help p""\x0F"));
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
                        sendMessage(tr("%1 draws 4 cards").arg(next->getColoredName()));
                        sendNotice(next->getName(), next->getDeck()->randCards(4, users->get(next->getName())->getColored()));
                        next->cantPlay();
                        end = true;
                    }
                    else if (id == "+2")
                    {
                        Player *next = players->get(nextPlayer());
                        sendMessage(tr("%1 draws 2 cards").arg(next->getColoredName()));
                        sendNotice(next->getName(), next->getDeck()->randCards(2, users->get(next->getName())->getColored()));
                        next->cantPlay();
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard(color, id);
                        end = true;
                    }
                    else if (id == "R")
                    {
                        if (players->size() == 2)
                            players->get(nextPlayer())->cantPlay();
                        else
                        {
                            sendMessage(tr("Turns are %1 reversed!").arg("\x16"));
                            inversed = !inversed;
                        }
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard(color, id);
                        end = true;
                    }
                    else if (id == "S")
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
                    sendMessage(tr("This color does not exist, %1").arg(curr->getColoredName()));
            }
            else
                sendMessage(tr("You can't play this card, %1").arg(curr->getColoredName()));
        }
        else
        {
            bool ok;
            QString color = QString(args.first()).toUpper(), id = QString(args.at(1)).toUpper();
            if (color != "R" && color != "G" && color != "B" && color != "Y")
                sendMessage(tr("This color does not exist, %1").arg(curr->getColoredName()));
            else if (id != "+4" && id != "+2" && id != "J" && id != "R" && id != "S" && !(id.toInt(&ok, 10) < 10 && ok && !id.contains("+")))
                sendMessage(tr("This card does not exist, %1").arg(curr->getColoredName()));
            else
                sendMessage(tr("You don't have this card, %1").arg(curr->getColoredName()));
        }
    }

    if (end && players->get(currPlayer)->getDeck()->size() == 1)
        sendMessage(tr("%1 is %2 !").arg(players->get(currPlayer)->getColoredName() + "\x16").arg("\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14""\x16"));
    else if (end && players->get(currPlayer)->getDeck()->size() == 0)
    {
        sendMessage(tr("%1 won the game!").arg(players->get(currPlayer)->getColoredName()));

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
                    else if (c->getId() == "R" || c->getId() == "S" || c->getId() == "+2")
                        points += 20;
                    else
                        points += c->getId().toInt();
                }
        }

        scores->setValue("Points/" + currPlayer, points);
        sendMessage(tr("%1 earned %2 points !").arg(players->get(currPlayer)->getColoredName()).arg(QString::number(points - origpoints)));

        sendMessage(" --- ");
        sendMessage(tr("Type %1 to show the scores").arg("\x02""!scores""\x0F"));
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
            sendMessage(tr("%1 skips his turn").arg(players->get(currPlayer)->getColoredName()));
            sendMessage(" --- ");
            sendMessage(tr("Last Card: %c"), lastCard);
            currPlayer = nextPlayer();
            sendMessage(tr("%1, it's your turn").arg(players->get(currPlayer)->getColoredName()));
            showCards();
        }
        else
        {
            sendMessage(" --- ");
            sendMessage(tr("Last card: %c"), lastCard);
            sendMessage(tr("%1, it's your turn").arg(players->get(currPlayer)->getColoredName()));
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
        sendMessage(tr("Scores are empty"));
        return;
    }

    sendMessage(tr("Scores:"));
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
        sendMessage((i == 9 ? "" : " ") + tr("%1. %2 : %3 (%4 points for %5 wins on %6 games played)").arg(QString::number(i + 1)).arg(users->contains(curr) ? users->get(curr)->getColoredName().insert(7, "\u200B") : "\x02" + curr + "\x0F").arg(QString::number((int)ratio)).arg(scores->value("Points/" + curr).toString()).arg(scores->value(curr).toString()).arg(scores->value("Total/" + curr).toString()));
        people.removeOne(curr);

        if (people.isEmpty())
            break;
    }

    flushMessages();
}

bool UNO::isOp(QString user)
{
    if (accesslist->contains(user) && users->get(user)->getHostname() == accesslist->value(user))
        return true;

    return network()->prefixToMode(users->get(user)->getMode()) == "o" || network()->prefixToMode(users->get(user)->getMode()) == "q" ? true : false;
}

bool UNO::startsWithMode(QString nick)
{
    QChar w = nick.at(0);
    if (w == network()->modeToPrefix("q") || w == network()->modeToPrefix("a") || w == network()->modeToPrefix("o") || w == network()->modeToPrefix("h") || w == network()->modeToPrefix("v"))
        return true;
    return false;
}
