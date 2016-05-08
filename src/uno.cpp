#include "uno.h"
#include "commands.cpp"

UNO::UNO(QCoreApplication *_parent) : IrcConnection(_parent)
{
    qsrand(QTime::currentTime().msec());

    verbose = 0;
    pick = new Cards(this);
    players = new Players(this);
    users = new Users;
    lastCard = new Card(NONE, "");
    identified = false, inGame = false, preGame = false, drawed = false, inversed = false, inPing = false, inVersion = false;
    updater = new Updater(qApp->applicationDirPath(), this);
    commands = new QHash<QString,fp>();
    commands->insert(tr("exit"), &UNO::exit);
    commands->insert(tr("kick"), &UNO::kick);
    commands->insert(tr("ban"), &UNO::ban);
    commands->insert(tr("unban"), &UNO::unban);
    commands->insert(tr("al"), &UNO::al);
    commands->insert(tr("merge"), &UNO::merge);
    commands->insert(tr("color"), &UNO::color);
    commands->insert(tr("nocolor"), &UNO::nocolor);
    commands->insert(tr("scores"), &UNO::scores);
    commands->insert(tr("help"), &UNO::help);
    commands->insert(tr("rules"), &UNO::rules);
    commands->insert(tr("version"), &UNO::version);
    commands->insert(tr("slaps"), &UNO::slap);
    commands->insert(tr("ping"), &UNO::ping);
    commands->insert(tr("list"), &UNO::list);
    commands->insert(tr("quit"), &UNO::quitGame);
    commands->insert(tr("join"), &UNO::joinGame);
    commands->insert(tr("begin"), &UNO::beginGame);
    commands->insert(tr("uno"), &UNO::uno);
    commands->insert(tr("draw"), &UNO::draw);
    commands->insert(tr("d", "Draw alias"), &UNO::draw);
    commands->insert(tr("end"), &UNO::endTurn);
    commands->insert(tr("e", "End alias"), &UNO::endTurn);
    commands->insert(tr("hand"), &UNO::hand);
    commands->insert(tr("cards"), &UNO::cardsGame);
    commands->insert(tr("p"), &UNO::play);

    log(INIT, tr("Loading ini files..."));

    settings = new QSettings(qApp->applicationDirPath() + "/UNObox/settings.ini", QSettings::IniFormat);
    settings->setIniCodec("UTF-8");

    slaps = new QSettings(qApp->applicationDirPath() + "/UNObox/slaps.ini", QSettings::IniFormat);
    slaps->setIniCodec("UTF-8");

    colors = new QSettings(qApp->applicationDirPath() + "/UNObox/colors.ini", QSettings::IniFormat);
    colors->setIniCodec("UTF-8");

    score = new QSettings(qApp->applicationDirPath() + "/UNObox/scores.ini", QSettings::IniFormat);
    score->setIniCodec("UTF-8");

    bans = new QSettings(qApp->applicationDirPath() + "/UNObox/bans.ini", QSettings::IniFormat);
    bans->setIniCodec("UTF-8");

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
    cnf = settings->value("commandnotfound", true).toBool();
    setReconnectDelay(5);

    log(INFO, tr("Server: %1 on port %2").arg(host()).arg(QString::number(port())));
    log(INFO, tr("SSL: %1").arg(isSecure() ? tr("true") : tr("false")));
    log(INFO, tr("Channel: %1").arg(chan));
    log(INFO, tr("Encoding: %1").arg(QString(encoding())));
    log(INFO, tr("Username: %1").arg(userName()));
    log(INFO, tr("Nickname: %1").arg(nickName()));
    log(INFO, tr("Realname: %1").arg(realName()));

    QVariantMap CtcpReplies;
    CtcpReplies.insert("VERSION", "\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14""\x16"" " + tr("[Update %1]").arg(COMMITDATE) + " - https://github.com/TheShayy/UNO ");
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
    score->sync();
    bans->sync();
    delete updater;
    delete pick;
    delete players;
    delete users;
    delete lastCard;
    delete settings;
    delete slaps;
    delete colors;
    delete score;
    delete bans;
    delete accesslist;
    delete commands;
}

void UNO::onSSLError()
{
    log(ERROR, tr("SSL error, set verbose level to 3 to show the details"));
}

void UNO::onConnect()
{
    if (!settings->value("nspassword", QString()).toString().isEmpty())
    {
        log(INFO, tr("Connected, logging in to NickServ"));
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
    case 400:
        log(ERROR, tr("Unknown error reported by %1: %2").arg(host()).arg(message->parameters().join(" ")));
        break;
    case 403:
        log(ERROR, tr("Channel %1 does not exist, exiting").arg(chan));
        qApp->exit(403);
    case 404:
        log(ERROR, tr("Cannot send message to %1").arg(chan));
        break;
    case 431:
        log(ERROR, tr("%1 said nickname is empty, exiting").arg(host()));
        qApp->exit(431);
    case 432:
        log(ERROR, tr("Erroneous nickname, exiting"));
        qApp->exit(432);
    case 433:
        log(ERROR, tr("%1 is already in use, exiting").arg(nickName()));
        qApp->exit(433);
    case 436:
        log(ERROR, tr("Nickname collision, exiting"));
        qApp->exit(436);
    case 471:
        log(ERROR, tr("%1 is full, exiting").arg(chan));
        qApp->exit(471);
    case 473:
        log(ERROR, tr("Invite only chans are not yet supported, exiting"));
        qApp->exit(473);
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
        sendMessage(tr("Wanna play? Try the %1 command!").arg("\x02""!" + tr("join") + "\x0F"));
    else if (!inGame)
        sendMessage(tr("Wanna play? Try the %1 command!").arg("\x02""!" + tr("help") + "\x0F"));
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
    if (!identified && message->target() == nickName() && message->mode().contains("+r"))
    {
        identified = true;
        log(INFO, tr("Identified. Joining %1").arg(chan));
        sendCommand(IrcCommand::createJoin(chan));
    }
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
    log(INFO, tr("%1 is now %2").arg(message->oldNick()).arg(message->newNick()));
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
    if (!identified && message->nick() == "NickServ" && message->content().contains("You are now ") && (message->content().contains("identified") || message->content().contains("recognized")))
    {
        identified = true;
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

void UNO::onUpdaterStep(QString step)
{
    if (step == "git")
        sendMessage(tr("Running git"), NULL, true);
    else if (step == "configure")
        sendMessage(tr("Running configure"), NULL, true);
    else if (step == "make")
        sendMessage(tr("Building"), NULL, true);
    else if (step == "files")
        sendMessage(tr("Replacing old files"), NULL, true);
}

void UNO::onUpdaterError(QString step)
{
    sendMessage(tr("Error processing step '%1'").arg(step), NULL, true);
}

void UNO::onUpdaterDone()
{
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
    sendRawNotice(to, players->get(nick)->getDeck()->toString(users->get(nick)->getColored()));
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
        sendMessage(tr("%1, it's your turn").arg(players->get(currPlayer)->getColoredName()));
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

void UNO::sendRawNotice(QString target, QString message)
{
    notices.insert(target, message);
}

void UNO::sendNotice(QString target, QString message, Card *card)
{
    if (users->contains(target) && !users->get(target)->getColored())
        notices.insert(target, "\x02""[UNO]""\x0F"" " + message.replace("%c", card != NULL ? card->toString(false) : ""));
    else
        notices.insert(target, "\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14 " + message.replace("\x02", "\x03""04,15").replace("\x0F", "\x03""00,14").replace("%c", card != NULL ? card->toString().replace("\x02", "\x03""04,15").replace("\x0F", "\x03""00,14") : "") + " ");
}

void UNO::sendMessage(QString message, Card *card, bool direct)
{
    QString ncmessage = "\x02""[UNO]""\x0F"" " + message;
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
                sendRawNotice(w->getNick(), ncmessage);
    }
}

void UNO::flushMessages()
{
    if (messages.isEmpty() && notices.isEmpty())
        return;

    socket()->waitForBytesWritten();

    foreach (QString w, messages)
        sendCommand(IrcCommand::createMessage(chan, w));

    QMultiMap<QString,QString> nts;
    foreach (QString w, notices.values())
        nts.insertMulti(notices.key(w), w);
    foreach (QString w, nts.values())
        sendCommand(IrcCommand::createNotice(notices.key(w), w));

    socket()->flush();
    messages.clear();
    notices.clear();
}

void UNO::command(QString nick, QString cmd, QStringList args)
{
    if (commands->contains(cmd))
        (this->*commands->value(cmd))(nick, args);
    else if (cnf)
        sendNotice(nick, tr("This command does not exist, %1").arg(users->contains(nick) ? users->get(nick)->getColoredName() : "\x02" + nick + "\x0F"));

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
    if (score->allKeys().isEmpty())
    {
        sendMessage(tr("Scores are empty"));
        return;
    }

    sendMessage(tr("Scores:"));
    QStringList people = score->allKeys();

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
            currratio = ((score->value("Points/" + w).toInt()/6) + (score->value(w).toInt()*105/score->value("Total/" + w).toInt()) + (score->value(w).toInt()*8)) / 4;
            if (currratio > ratio)
            {
                curr = w;
                ratio = currratio;
            }
        }
        sendMessage((i == 9 ? "" : " ") + tr("%1. %2 : %3 (%4 points for %5 wins on %6 games played)").arg(QString::number(i + 1)).arg(users->contains(curr) ? users->get(curr)->getColoredName().insert(7, "\u200B") : "\x02" + curr + "\x0F").arg(QString::number((int)ratio)).arg(score->value("Points/" + curr).toString()).arg(score->value(curr).toString()).arg(score->value("Total/" + curr).toString()));
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
