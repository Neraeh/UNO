#ifndef COMMANDS_CPP
#define COMMANDS_CPP

#include "uno.h" // For syntax highlighting, useless for building

void UNO::exit(QString nick, QStringList args)
{
    if (!isOp(nick))
        return;

    if (args.isEmpty())
        qApp->exit();
    else
        qApp->exit(args.first().toInt());
}

#ifndef Q_OS_WIN
void UNO::update(QString nick, QStringList args)
{
    Q_UNUSED(args)

    if (!isOp(nick))
        return;

    sendMessage(tr("Updating %1").arg(nickName()), NULL, true);
    updater->start();
}
#endif

void UNO::changeTrigger(QString nick, QStringList args)
{
    if (!isOp(nick) || args.isEmpty())
        return;

    if (args.size() > 1)
        sendNotice(nick, tr("Usage: %1").arg(QString(trigger) + " " + tr("trigger") + " <trigger>"));
    else if (args.first().size() > 1)
        sendNotice(nick, tr("The trigger can only be a single character"));
    else {
        settings->setValue("trigger", trigger = args.first().at(0));
        sendNotice(nick, tr("The trigger is now %1").arg(trigger));
    }
}

void UNO::kick(QString nick, QStringList args)
{
    if (!isOp(nick) || args.isEmpty())
        return;

    remPlayer(args.first());
}

void UNO::ban(QString nick, QStringList args)
{
    if (!isOp(nick) || args.isEmpty())
        return;

    if (users->contains(args.first()))
    {
        bans->setValue(args.first(), users->get(args.first())->getHostname());
        sendMessage(tr("%1 is now banned").arg(users->get(args.first())->getColoredName()));
    }
    else
        sendNotice(nick, tr("%1 was not found").arg("\x02" + args.first() + "\x0F"));
}

void UNO::unban(QString nick, QStringList args)
{
    if (!isOp(nick) || args.isEmpty())
        return;

    if (users->contains(args.first()))
    {
        bans->remove(args.first());
        sendMessage(tr("%1 is no longer banned").arg(users->get(args.first())->getColoredName()));
    }
    else
        sendNotice(nick, tr("%1 was not found").arg("\x02" + args.first() + "\x0F"));
}

void UNO::al(QString nick, QStringList args)
{
    if (!accesslist->contains(nick))
        return;

    if (args.isEmpty())
        sendNotice(nick, "list, add, del, host");
    else if (args.first() == "add")
    {
        if (args.size() == 2)
        {
            if (accesslist->contains(args.at(1)))
                sendNotice(nick, tr("%1 is already in the access list").arg(users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F"));
            else if (users->contains(args.at(1)))
            {
                accesslist->setValue(args.at(1), users->get(args.at(1))->getHostname());
                sendNotice(nick, tr("%1 has been added").arg(users->get(args.at(1))->getColoredName()));
            }
            else
                sendNotice(nick, tr("%1 was not found").arg("\x02" + args.at(1) + "\x0F"));
        }
        else
            sendNotice(nick, tr("Usage: %1").arg(QString(trigger) + tr("al") + " add <%2>").arg(tr("nick")));
    }
    else if (args.first() == "del" && args.size() == 2)
    {
        if (args.size() == 2)
        {
            if (accesslist->contains(args.at(1)))
            {
                accesslist->remove(args.at(1));
                sendNotice(nick, tr("%1 has been removed").arg(users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F"));
            }
            else
                sendNotice(nick, tr("%1 is not in the access list").arg(users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F"));
        }
        else
            sendNotice(nick, tr("Usage: %1").arg(QString(trigger) + tr("al") + " del <%2>").arg(tr("nick")));
    }
    else if (args.first() == "list")
    {
        sendNotice(nick, tr("In the access list: %1").arg(accesslist->allKeys().join(", ")));
    }
    else if (args.first() == "host")
    {
        if (args.size() == 2)
        {
            if (accesslist->contains(args.at(1)))
                sendNotice(nick, users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F" + ": " + accesslist->value(args.at(1)).toString());
            else
                sendNotice(nick, tr("%1 is not in the access list").arg(users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F"));
        }
        else
            sendNotice(nick, tr("Usage: %1").arg(QString(trigger) + tr("al") + " host %2").arg(tr("nick")));
    }
}

void UNO::merge(QString nick, QStringList args)
{
    if (!isOp(nick) || args.size() < 2)
        return;

    if (!score->contains(args.first()))
        sendNotice(nick, tr("%1 was not found").arg(users->contains(args.first()) ? users->get(args.first())->getColoredName() : "\x02" + args.first() + "\x0F"));
    else if (!score->contains(args.at(1)))
        sendNotice(nick, tr("%1 was not found").arg(users->contains(args.first()) ? users->get(args.first())->getColoredName() : "\x02" + args.first() + "\x0F"));
    else
    {
        score->setValue(args.first(), score->value(args.first()).toInt() + score->value(args.at(1)).toInt());
        score->remove(args.at(1));
        score->setValue("Points/" + args.first(), score->value("Points/" + args.first()).toInt() + score->value("Points/" + args.at(1)).toInt());
        score->remove("Points/" + args.at(1));
        score->setValue("Total/" + args.first(), score->value("Total/" + args.first()).toInt() + score->value("Total/" + args.at(1)).toInt());
        score->remove("Total/" + args.at(1));
        sendMessage(tr("%1 and %2 have successfully been merged into %1").arg(users->contains(args.first()) ? users->get(args.first())->getColoredName() : "\x02" + args.first() + "\x0F").arg(users->contains(args.at(1)) ? users->get(args.at(1))->getColoredName() : "\x02" + args.at(1) + "\x0F"));
    }
}

void UNO::color(QString nick, QStringList args)
{
    if (args.isEmpty() || args.size() > 1)
    {
        sendNotice(nick, tr("Usage : %1%2 number").arg(trigger).arg(tr("color")));
        sendNotice(nick, tr("Available numbers : %1").arg("\x03""022  ""\x03""033  ""\x03""044  ""\x03""055  ""\x03""066  ""\x03""077  ""\x03""088  ""\x03""099  ""\x03""1010  ""\x03""1111  ""\x03""1212  ""\x03""1313"));
        sendNotice(nick, tr("Current color : %1").arg(users->get(nick)->getColoredName()));
    }
    else if (args.first().toInt() >= 2 && args.first().toInt() <= 13 && args.size() == 1)
    {
        if (players->contains(nick) && (inGame || preGame))
            sendNotice(nick, tr("You can't change your color during a game, %1").arg(players->get(nick)->getColoredName()));
        else
        {
            users->get(nick)->setColor(args.first().toShort());
            colors->setValue(nick, args.first().toShort());
            sendMessage(tr("You changed your color, %1").arg(users->get(nick)->getColoredName()));
        }
    }
    else
        sendNotice(nick, tr("%1 is not a valid color").arg(args.first()));
}

void UNO::nocolor(QString nick, QStringList args)
{
    Q_UNUSED(args)

    users->get(nick)->setColored(!users->get(nick)->getColored());
    sendNotice(nick, tr("Color compatibility %1").arg(users->get(nick)->getColored() ? tr("disabled") : tr("enabled")));
}

void UNO::scores(QString nick, QStringList args)
{
    Q_UNUSED(nick)
    Q_UNUSED(args)

    showScores();
}

void UNO::help(QString nick, QStringList args)
{
    if (args.isEmpty())
    {
        sendNotice(nick, QString(trigger) + tr("version") + ",  " + QString(trigger) + tr("rules") + ", " + QString(trigger) + tr("color") + ", " + QString(trigger) + tr("nocolor") + ", " + QString(trigger) + tr("scores") + ", " + QString(trigger) + tr("list") + ", " + QString(trigger) + tr("quit") + ", " + QString(trigger) + tr("uno") + ", " + QString(trigger) + tr("join") + ", " + QString(trigger) + tr("begin") + ", " + QString(trigger) + tr("p") + ", " + QString(trigger) + tr("draw") + ", " + QString(trigger) + tr("end") + ", " + QString(trigger) + tr("hand") + ", " + QString(trigger) + tr("cards"));
        sendNotice(nick, tr("Type %1 to learn more about a command (example: %2)").arg("\x02" + QString(trigger) + tr("help") + " <" + tr("command") + ">""\x0F").arg(QString(trigger) + tr("help") + " " + tr("join")));
        sendNotice(nick, tr("Colors: %1 %2 %3 %4").arg("\x02""\x03""01,04""[" + QObject::tr("R", "Red letter ingame (translate to first letter of the color in your language)") + "] " + tr("red") + "\x0F").arg("\x02""\x03""01,03""[" + QObject::tr("G", "Green letter ingame (translate to first letter of the color in your language)") + "] " + tr("green") + "\x0F").arg("\x02""\x03""01,11""[" + QObject::tr("B", "Blue letter ingame (translate to first letter of the color in your language)") + "] " + tr("blue") + "\x0F").arg("\x02""\x03""01,08""[" + QObject::tr("Y", "Yellow letter ingame (translate to first letter of the color in your language)") + "] " + tr("yellow") + "\x0F"));
    }
    else if (args.first() == tr("uno"))
        sendNotice(nick, tr("%1 %2 : create a new game").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("uno")));
    else if (args.first() == tr("p"))
    {
        sendNotice(nick, tr("%1 %2").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("p")));
        sendNotice(nick, tr("Usage : %1 color card").arg(trigger + tr("p")));
        sendNotice(nick, tr("Example : to play a %c you should type %1 %2 7").arg(trigger + tr("p")).arg(tr("G", "Green letter ingame (translate to first letter of the color in your language)")), new Card(GREEN, "7"));
    }
    else if (args.first() == tr("join"))
        sendNotice(nick, tr("%1 %2 : join the current game").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("join")));
    else if (args.first() == tr("quit"))
        sendNotice(nick, tr("%1 %2 : leave the game").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("quit")));
    else if (args.first() == tr("begin"))
        sendNotice(nick, tr("%1 %2 : begin the game").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("begin")));
    else if (args.first() == tr("list"))
        sendNotice(nick, tr("%1 %2 : display players list").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("join")));
    else if (args.first() == tr("draw"))
        sendNotice(nick, tr("%1 %2 : draw a card").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("draw")));
    else if (args.first() == tr("end"))
        sendNotice(nick, tr("%1 %2 : skip your turn").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("end")));
    else if (args.first() == tr("hand"))
        sendNotice(nick, tr("%1 %2 : display your cards and the last played card").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("hand")));
    else if (args.first() == tr("cards"))
        sendNotice(nick, tr("%1 %2 : display the remaining cards count and the players cards count").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("cards")));
    else if (args.first() == tr("rules"))
        sendNotice(nick, tr("%1 %2 : send a link to the rules").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("rules")));
    else if (args.first() == tr("ping"))
        sendNotice(nick, tr("%1 %2 : display the latency between %2 and you").arg("\x02" + tr("Help :") + "\x0F").arg("\x02" + nickName() + "\x0F").arg(trigger + tr("ping")));
    else if (args.first() == tr("version"))
        sendNotice(nick, tr("%1 %2 : display the client version of the given user").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("version")));
    else if (args.first() == tr("slaps"))
        command(nickName(), tr("slaps"), QStringList() << nick);
    else if (args.first() == tr("color"))
        sendNotice(nick, tr("%1 %2 : choose your color").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("color")));
    else if (args.first() == tr("nocolor"))
        sendNotice(nick, tr("%1 %2 : enable/disable the color compatibilty mode").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("nocolor")));
    else if (args.first() == tr("scores"))
        sendNotice(nick, tr("%1 %2 : display scores").arg("\x02" + tr("Help :") + "\x0F").arg(trigger + tr("scores")));
    else
        sendNotice(nick, tr("This command does not exist, %1").arg(users->get(nick)->getColoredName()));
}

void UNO::rules(QString nick, QStringList args)
{
    Q_UNUSED(nick)
    Q_UNUSED(args)

    sendNotice(nick, tr("%1 http://tuxange.org/unorules/").arg("\x02" + tr("Rules (french) :") + "\x0F"));
}

void UNO::version(QString nick, QStringList args)
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

void UNO::slap(QString nick, QStringList args)
{
    if (args.isEmpty())
        args.append(users->rand()->getNick());

    sendCommand(IrcCommand::createMessage(chan, "\x02""\x03""00,14 " + users->get(nick)->getColoredName() + " " + slaps->value(QString::number(qrand() % slaps->allKeys().size())).toString().replace("%s", (users->contains(args.first()) ? users->get(args.first())->getColoredName() : "\x03""04,15" + args.first() + "\x03""00,14")) + " "));
}

void UNO::ping(QString nick, QStringList args)
{
    Q_UNUSED(args)

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

void UNO::list(QString nick, QStringList args)
{
    Q_UNUSED(args)

    if (inGame || preGame)
        sendNotice(nick, tr("In the game: %1").arg(inGame ? showTurns() : players->list()));
    else
        sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
}

void UNO::quitGame(QString nick, QStringList args)
{
    Q_UNUSED(args)
    if (players->contains(nick))
        remPlayer(nick);
    else if (!inGame)
        sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
    else
        sendNotice(nick, tr("You are not in this game, %1").arg(users->get(nick)->getColoredName()));
}

void UNO::joinGame(QString nick, QStringList args)
{
    Q_UNUSED(args)
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
            sendNotice(nick, tr("You are already in this game, %1").arg(players->get(nick)->getColoredName()));
    }
    else if (inGame)
        sendNotice(nick, tr("You can't join a launched game, %1").arg(users->get(nick)->getColoredName()));
    else
        sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
}

void UNO::beginGame(QString nick, QStringList args)
{
    Q_UNUSED(args)
    if (!players->contains(nick) && preGame)
        sendNotice(nick, tr("You are not in this game, %1").arg(users->get(nick)->getColoredName()));
    else if (players->size() > 1 && preGame)
    {
        pick->randomize();
        do
        {
            lastCard = pick->pick();
        }
        while (lastCard->getId() == "+4");
        foreach (Player *w, players->getList())
            w->getDeck()->init();
        sendMessage(" --- ");
        sendMessage(tr("Last card: %c"), lastCard);
        currPlayer = players->rand()->getName();

        if (lastCard->getId() == "+2")
        {
            sendMessage(tr("%1 draws %2 cards").arg(players->get(currPlayer)->getColoredName()).arg("2"));
            sendRawNotice(currPlayer, players->get(currPlayer)->getDeck()->randCards(2, users->get(currPlayer)->getColored()));
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
        sendNotice(nick, tr("There is not enough players to launch the game, %1").arg(players->get(nick)->getColoredName()));
    else if (inGame)
        sendNotice(nick, tr("A game is already launched, %1").arg(users->get(nick)->getColoredName()));
    else
        sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
}

void UNO::uno(QString nick, QStringList args)
{
    Q_UNUSED(args)
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
        sendNotice(nick, tr("A game is already launched, %1").arg(users->get(nick)->getColoredName()));
    else
        sendNotice(nick, tr("A game is already being created, %1").arg(users->get(nick)->getColoredName()));
}

void UNO::draw(QString nick, QStringList args)
{
    Q_UNUSED(args)
    if (!inGame)
        sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
    else if (!drawed && currPlayer == nick)
    {
        sendRawNotice(currPlayer, players->get(currPlayer)->getDeck()->randCards(1));
        sendMessage(tr("Last card: %c"), lastCard);
        drawed = true;
    }
    else if (currPlayer == nick)
        sendMessage(tr("You already drew, %1").arg(players->get(nick)->getColoredName()));
    else
        sendMessage(tr("%1, it's your turn").arg(players->get(currPlayer)->getColoredName()));
}

void UNO::endTurn(QString nick, QStringList args)
{
    Q_UNUSED(args)
    bool end = false;

    if (!inGame)
        sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
    else if (drawed && currPlayer == nick)
        end = true;
    else if (currPlayer == nick)
        sendMessage(tr("You did not draw, %1").arg(players->get(currPlayer)->getColoredName()));
    else
        sendMessage(tr("%1, it's your turn").arg(players->get(currPlayer)->getColoredName()));

    if (end)
        turnEnd();
}

void UNO::hand(QString nick, QStringList args)
{
    if (!inGame)
        sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
    else if (!isOp(nick) && !args.isEmpty())
        sendNotice(nick, tr("You can't see the other players cards, %1").arg(users->get(nick)->getColoredName()));
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
            sendNotice(nick, tr("%1 was not found, %2").arg("\x02" + args.at(0) + "\x0F").arg(users->get(nick)->getColoredName()));
        // FLUSH
    }
}

void UNO::cardsGame(QString nick, QStringList args)
{
    Q_UNUSED(args)
    if (!inGame)
        sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
    else
    {
        sendNotice(nick, tr("%1 cards remaining").arg(QString::number(pick->size())));
        foreach (Player *w, players->getList())
            sendNotice(nick, tr("%1 has %2 cards").arg(w->getColoredName()).arg(QString::number(w->getDeck()->size())));
    }
}

void UNO::play(QString nick, QStringList args)
{
    bool end = false;
    Player *curr = players->get(currPlayer);
    if (!inGame)
        sendMessage(tr("There is no game, %1").arg(users->get(nick)->getColoredName()));
    else if (currPlayer != nick)
        sendMessage(tr("%1, it's your turn").arg(curr->getColoredName()));
    else if (args.size() < 2)
        sendMessage(tr("%1 to learn how to use it").arg("\x02" + QString(trigger) + tr("help") + " " + tr("p") + "\x0F"));
    else if (curr->getDeck()->contains(new Card(args.at(0), args.at(1))) || curr->getDeck()->contains(new Card(NONE, args.at(1))))
    {
        Color color = Card::toColor(args.at(0));
        QString id = QString(args.at(1)).toUpper();
        if (lastCard->getColor() == color || lastCard->getId() == id || lastCard->getColor() == NONE || id == "J" || id == "+4")
        {
            if (color != NONE)
            {
                if (id == "J")
                {
                    lastCard = new Card(color, id);
                    curr->getDeck()->remCard(NONE, id);
                    end = true;
                }
                else if (id == "+4")
                {
                    lastCard = new Card(color, id);
                    curr->getDeck()->remCard(NONE, id);
                    Player *next = players->get(nextPlayer());
                    sendMessage(tr("%1 draws %2 cards").arg(next->getColoredName()).arg("4"));
                    sendRawNotice(next->getName(), next->getDeck()->randCards(4, users->get(next->getName())->getColored()));
                    next->cantPlay();
                    end = true;
                }
                else if (id == "+2")
                {
                    Player *next = players->get(nextPlayer());
                    sendMessage(tr("%1 draws %2 cards").arg(next->getColoredName()).arg("2"));
                    sendRawNotice(next->getName(), next->getDeck()->randCards(2, users->get(next->getName())->getColored()));
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
        Color color = Card::toColor(args.first());
        QString id = QString(args.at(1)).toUpper();
        if (color != RED && color != GREEN && color != BLUE && color != YELLOW)
            sendMessage(tr("This color does not exist, %1").arg(curr->getColoredName()));
        else if (id != "+4" && id != "+2" && id != "J" && id != "R" && id != "S" && !(id.toInt(&ok, 10) < 10 && ok && !id.contains("+")))
            sendMessage(tr("This card does not exist, %1").arg(curr->getColoredName()));
        else
            sendMessage(tr("You don't have this card, %1").arg(curr->getColoredName()));
    }

    if (end)
        turnEnd();
}

void UNO::turnEnd()
{
    if (players->get(currPlayer)->getDeck()->size() == 1)
        sendMessage(tr("%1 is %2 !").arg(players->get(currPlayer)->getColoredName() + "\x16").arg("\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14""\x16"));
    else if (players->get(currPlayer)->getDeck()->size() == 0)
    {
        sendMessage(tr("%1 won the game!").arg(players->get(currPlayer)->getColoredName()));

        score->setValue(currPlayer, score->value(currPlayer, 0).toInt() + 1);
        int points = score->value("Points/" + currPlayer, 0).toInt(), origpoints = points;

        foreach (Player *w, players->getList())
        {
            score->setValue("Total/" + w->getName(), score->value("Total/" + w->getName(), 0).toInt() + 1);
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

        score->setValue("Points/" + currPlayer, points);
        sendMessage(tr("%1 earned %2 points !").arg(players->get(currPlayer)->getColoredName()).arg(QString::number(points - origpoints)));

        sendMessage(" --- ");
        sendMessage(tr("Type %1 to show the scores").arg("\x02" + QString(trigger) + tr("scores") + "\x0F"));
        flushMessages();
        clear();
        return;
    }

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

#endif
