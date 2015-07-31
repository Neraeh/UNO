#include "uno.h"

UNO::UNO(QCoreApplication *_parent) : IrcConnection(_parent)
{
    cards = new Cards;
    players = new Players;
    lastCard = new Card("", "");
    inGame = false, preGame = false, drawed = false, inversed = false, inPing = false, inVersion = false;

    settings = new QSettings(qApp->applicationDirPath() + "/ini/settings.ini", QSettings::IniFormat);
    settings->setIniCodec("UTF-8");

    slaps = new QSettings(qApp->applicationDirPath() + "/ini/slaps.ini", QSettings::IniFormat);
    slaps->setIniCodec("UTF-8");

    qputenv("IRC_DEBUG", settings->value("debug", "0").toByteArray());

    setServers(QStringList(settings->value("server", "irc.t411.io").toString()));
    chan = settings->value("chan", "#uno").toString();
    setEncoding(settings->value("encoding", "UTF-8").toByteArray());
    setUserName(settings->value("username", "UNO").toString());
    setNickName(settings->value("nickname", "UNO").toString());
    setRealName(settings->value("realname", "UNO").toString());
    setReconnectDelay(5);
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
    delete cards;
    delete players;
    delete lastCard;
    delete settings;
    delete slaps;
}

void UNO::onConnect()
{
    if (!settings->value("nspassword", QString()).toString().isEmpty())
        sendCommand(IrcCommand::createMessage("NickServ", "IDENTIFY " + settings->value("nspassword").toString()));
    sendCommand(IrcCommand::createJoin(chan));
}

void UNO::onMessage(IrcPrivateMessage *message)
{
    if (message->target() == nickName() || message->isOwn())
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
    sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + message->nick()));
    if (preGame)
        sendMessage("Envie de jouer au UNO ? Tapez ""\x02""UNO""\x0F"" pour rejoindre la partie en préparation !");
    else if (!inGame)
        sendMessage("Envie de jouer au UNO ? Tapez ""\x02""!aide""\x0F"" pour afficher la liste des commandes");
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
        modes.insert(startsWithMode(w) ? w.mid(1) : w, startsWithMode(w) ? w.at(0) : QString());
}

void UNO::onNick(IrcNickMessage *message)
{
    sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + message->newNick()));
    modes.insert(message->newNick(), modes.value(message->oldNick()));
    modes.remove(message->oldNick());
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

void UNO::onNotice(IrcNoticeMessage *message)
{
    if (message->content().split(" ").size() > 3 && message->nick() == "NickServ" && message->content().startsWith("STATUS") && (message->content().split(" ").at(3) == "Shayy" || message->content().split(" ").at(3) == "TuxAnge" || message->content().split(" ").at(3) == "Feeling"))
        sendCommand(IrcCommand::createMode(chan, "+o", message->content().split(" ").at(3)));
    else if (message->content().startsWith("VERSION"))
    {
        sendCommand(IrcCommand::createMessage(chan, "\x02" + message->nick() + "\x0F"" utilise:" + message->content().mid(message->content().indexOf(" "))));
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
            sendCommand(IrcCommand::createMessage(chan, "\x03""01,03" + green + "\x03""01,04" + red + "\x0F"" ""\x02" + currPing + "\x0F"": " + QString::number(pingTime) + "ms"));
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
    modes.remove(message->nick());
}

void UNO::onQuit(IrcQuitMessage *message)
{
    remPlayer(message->nick());
    modes.remove(message->nick());
}

void UNO::pingTimeout()
{
    if (!inPing)
        return;
    sendCommand(IrcCommand::createMessage(chan, "\x02" + currPing + "\x0F"" a dépassé le délai autorisé"));
    inPing = false;
}

void UNO::versionTimeout(QString nick)
{
    if (!inVersion)
        return;
    sendCommand(IrcCommand::createMessage(chan, "\x02" + nick + "\x0F"" a dépassé le délai autorisé"));
    inVersion = false;
}

void UNO::showCards(QString nick, QString to)
{
    if (nick.isEmpty())
        nick = currPlayer;
    if (to.isEmpty())
        to = nick;
    sendCommand(IrcCommand::createNotice(to, players->getPlayer(nick)->getDeck()->toString()));
}

QString UNO::nextPlayer()
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
        sendMessage(players->getPlayer(nick)->getColoredName() + " a quitté la partie");
        players->remove(nick);
        turns.removeOne(nick);

        if (players->size() == 1 && inGame)
        {
            sendMessage(players->getPlayer(players->first())->getColoredName() + " a gagné la partie !");
            clear();
        }
        else if (nick == currPlayer && inGame)
        {
            currPlayer = nextPlayer();
            sendMessage("C'est donc au tour de " + players->getPlayer(currPlayer)->getColoredName());
        }
        else if (players->size() == 0 && preGame)
        {
            sendMessage("Plus aucun joueur, la préparation de la partie est annulée");
            clear();
        }
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
    delete cards;
    cards = new Cards();
}

void UNO::sendMessage(QString message)
{
    message.replace("\x02", "\x03""04,15");
    message.replace("\x0F", "\x03""00,14");
    QString msg = "\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14 " + message;
    int len;
    do {
        msg += " ";
        QString lenmsg = msg;
        for (int i = 0; i < lenmsg.length(); i++)
        {
            while (lenmsg.at(i) == QString("\x02") || lenmsg.at(i) == QString("\x0F") || lenmsg.at(i) == QString("\x16") || lenmsg.at(i) == QString ("\x03"))
            {
                if (lenmsg.at(i) == QString("\x02") || lenmsg.at(i) == QString("\x0F") || lenmsg.at(i) == QString("\x16"))
                    lenmsg.remove(i, 1);
                else if (lenmsg.at(i) == QString("\x03"))
                    lenmsg.remove(i, 6);
            }
        }
        len = lenmsg.length();
    } while (len < 100);
    sendCommand(IrcCommand::createMessage(chan, msg));
}

void UNO::command(QString nick, QString cmd, QStringList args)
{
    bool end = false;

    if (isOp(nick))
    {
        if (cmd == "exit")
            qApp->quit();
        else if (cmd == "sendraw")
            sendRaw(args.join(" "));
        else if (cmd == "kick" && !args.isEmpty())
            remPlayer(args.first());
    }
    else if (cmd == "op")
        sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + nick));

    if (cmd == "aide" || cmd == "help")
    {
        if (args.isEmpty())
        {
            sendMessage("\x02""Tout le temps :""\x0F"" !ping, !version, !slaps, !regles");
            sendMessage("\x02""Avant et pendant la partie :""\x0F"" !liste, !quitter");
            sendMessage("\x02""Avant la partie :""\x0F"" !uno, !rejoindre, !commencer");
            sendMessage("\x02""Pendant la partie :""\x0F"" !j, !pioche, !fin, !main, !cartes");
            sendMessage("\x02""!aide commande""\x0F"" pour avoir des détails sur une commande (exemple: !aide rejoindre)");
        }
        else if (args.first() == "uno")
            sendMessage("\x02""Aide :""\x0F"" !uno : commencer la préparation d'une nouvelle partie");
        else if (args.first() == "j")
        {
            sendMessage("\x02""Aide :""\x0F"" !j");
            sendMessage("Utilisation : !j couleur carte");
            sendMessage("Exemple : pour jouer un " + Card("V", "7").toString() + " il faut écrire !j v 7");
        }
        else if (args.first() == "rejoindre")
            sendMessage("\x02""Aide :""\x0F"" !rejoindre : rejoindre la partie en préparation");
        else if (args.first() == "quitter")
            sendMessage("\x02""Aide :""\x0F"" !quitter : quitter la partie");
        else if (args.first() == "commencer")
            sendMessage("\x02""Aide :""\x0F"" !commencer : commencer la partie en préparation");
        else if (args.first() == "liste" || args.first() == "list")
            sendMessage("\x02""Aide :""\x0F"" !liste : affiche la liste des joueurs dans la partie");
        else if (args.first() == "pioche")
            sendMessage("\x02""Aide :""\x0F"" !pioche : piocher une carte");
        else if (args.first() == "fin" || args.first() == "f")
            sendMessage("\x02""Aide :""\x0F"" !fin : passer son tour");
        else if (args.first() == "main" || args.first() == "m")
            sendMessage("\x02""Aide :""\x0F"" !main : affiche les cartes dans votre main et la carte visible");
        else if (args.first() == "cartes")
            sendMessage("\x02""Aide :""\x0F"" !cartes : affiche le nombre de cartes restantes dans la pioche");
        else if (args.first() == "regles" || args.first() == "rules")
            sendMessage("\x02""Aide :""\x0F"" !regles : affiche les règles du jeu");
        else if (args.first() == "ping")
            sendMessage("\x02""Aide :""\x0F"" !ping : affiche la latence entre ""\x02" + nickName() + "\x0F"" et vous");
        else if (args.first() == "version")
            sendMessage("\x02""Aide :""\x0F"" !version : affiche la version du client de l'utilisateur donné");
        else if (args.first() == "slaps")
            command(nickName(), "slaps", QStringList() << nick);
        else
            sendMessage("Cette commande n'existe pas, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick + "\x0F"));
    }
    else if (cmd == "regles" || cmd == "rules")
        sendMessage("\x02""Règles :""\x0F"" http://tuxange.org/unorules/");
    else if (cmd == "version")
    {
        if (args.isEmpty())
            args.append(nick);

        if (modes.contains(args.first()))
        {
            sendCommand(IrcCommand::createVersion(args.first()));
            QTimer::singleShot(20000, this, SLOT(versionTimeout(nick)));
            inVersion = true;
        }
        else
            sendCommand(IrcCommand::createMessage(chan, "\x02" + args.first() + "\x0F"" n'a pas été trouvé"));
    }
    else if (cmd == "slaps")
    {
        if (args.isEmpty())
            args.append(nickName());
        qsrand((uint)QTime::currentTime().msec());
        sendCommand(IrcCommand::createMessage(chan, "\x02" + nick + "\x0F"" " + slaps->value(QString::number(qrand() % 23)).toString().replace("%s", "\x02" + args.first() + "\x0F")));
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
            sendCommand(IrcCommand::createMessage(chan, "Un ping est déjà en cours, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick)));
    }
    else if (cmd == "liste" || cmd == "list")
    {
        if (inGame || preGame)
            sendMessage("Dans la partie: " + players->list());
        else
            sendMessage("Il n'y a pas de partie en cours, ""\x02" + nick + "\x0F");
    }
    else if (cmd == "quitter")
    {
        if (players->contains(nick))
            remPlayer(nick);
        else if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, ""\x02" + nick + "\x0F");
        else
            sendMessage("Vous n'êtes pas dans cette partie, ""\x02" + nick + "\x0F");
    }
    else if (cmd == "rejoindre")
    {
        if (preGame)
        {
            if (!players->contains(nick))
            {
                Player *p = new Player(nick, this);
                players->add(p);
                turns.append(nick);
                sendMessage(p->getColoredName() + " a rejoint la partie");
                sendMessage("Il y a " + QString::number(players->size()) + " joueurs dans la partie");
            }
            else
                sendMessage("Vous êtes déjà dans la partie, " + players->getPlayer(nick)->getColoredName());
        }
        else if (inGame)
            sendMessage("Impossible de rejoindre une partie en cours de jeu, ""\x02" + nick + "\x0F");
        else
            sendMessage("Il n'y a pas de partie en préparation, ""\x02" + nick + "\x0F");
    }
    else if (cmd == "commencer")
    {
        if (!players->contains(nick) && preGame)
            sendMessage("Vous n'êtes pas dans cette partie, ""\x02" + nick + "\x0F");
        else if (players->size() > 1 && preGame)
        {;
            QTime time = QTime::currentTime();
            qsrand((uint)time.msec());
            int rand;
            do { rand = qrand() % cards->size() - 1; } while (cards->get(rand)->getId() == "+4");
            lastCard = cards->get(rand);
            cards->remove(rand);
            foreach (Player *w, players->getList())
                w->getDeck()->init();
            sendCommand(IrcCommand::createMessage(chan, " "));
            sendMessage("Carte visible : " + lastCard->toString());
            currPlayer = players->rand()->getName();

            if (lastCard->getId() == "+2")
            {
                sendMessage(players->getPlayer(currPlayer)->getColoredName() + " pioche 2 cartes");
                sendCommand(IrcCommand::createNotice(currPlayer, players->getPlayer(currPlayer)->getDeck()->randCards(2)));
                sendMessage(players->getPlayer(currPlayer)->getColoredName() + " passe son tour");
                currPlayer = nextPlayer();
            }
            else if (lastCard->getId() == "I")
            {
                sendMessage("Le sens du jeu est ""\x16""inversé");
                inversed = true;
            }
            else if (lastCard->getId() == "P")
            {
                sendMessage(players->getPlayer(currPlayer)->getColoredName() + " passe son tour");
                currPlayer = nextPlayer();
            }

            sendMessage("C'est au tour de " + players->getPlayer(currPlayer)->getColoredName());

            foreach (Player *w, players->getList())
                sendCommand(IrcCommand::createNotice(w->getName(), w->getDeck()->toString()));

            inGame = true;
            preGame = false;
        }
        else if (preGame)
            sendMessage("Il n'y a pas assez de joueurs pour commencer la partie, " + players->getPlayer(nick)->getColoredName());
        else if (inGame)
            sendMessage("Une partie est déjà en cours, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick));
        else
            sendMessage("Il n'y a pas de partie en préparation, ""\x02" + nick + "\x0F");
    }
    else if (cmd == "uno")
    {
        if (!inGame && !preGame)
        {
            preGame = true;
            Player *p = new Player(nick, this);
            players->add(p);
            turns.append(nick);
            sendMessage(p->getColoredName() + " a créé une nouvelle partie");
            sendMessage(p->getColoredName() + " a rejoint la partie");
            sendMessage("Il y a 1 joueur dans la partie");
        }
        else if (inGame)
            sendMessage("Une partie est déjà en cours, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick + "\x0F"));
        else // preGame
            sendMessage("Une partie est déjà en préparation, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick + "\x0F"));
    }
    else if (cmd == "pioche")
    {
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, ""\x02" + nick + "\x0F");
        else if (!drawed && currPlayer == nick)
        {
            sendCommand(IrcCommand::createNotice(currPlayer, players->getPlayer(currPlayer)->getDeck()->randCards(1)));
            sendMessage("Carte visible : " + lastCard->toString());
            drawed = true;
        }
        else if (currPlayer == nick)
            sendMessage("Vous avez déjà pioché, " + players->getPlayer(nick)->getColoredName());
        else
            sendMessage("C'est au tour de " + players->getPlayer(currPlayer)->getColoredName());
    }
    else if (cmd == "fin" || cmd == "f")
    {
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, ""\x02" + nick + "\x0F");
        else if (drawed && currPlayer == nick)
            end = true;
        else if (currPlayer == nick)
            sendMessage("Vous n'avez pas pioché, " + players->getPlayer(currPlayer)->getColoredName());
        else
            sendMessage("C'est au tour de " + players->getPlayer(currPlayer)->getColoredName());
    }
    else if (cmd == "main")
    {
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, ""\x02" + nick + "\x0F");
        else if (!isOp(nick) && !args.isEmpty())
            sendMessage("Vous n'avez pas le droit de voir les cartes des autres joueurs, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick + "\x0F"));
        else if (args.isEmpty())
        {
            sendMessage("Carte visible : " + lastCard->toString());
            sendMessage("C'est au tour de " + players->getPlayer(currPlayer)->getColoredName());
            if (players->contains(nick)) showCards(nick);
        }
        else if (isOp(nick))
        {
            if (players->contains(args.at(0)))
                showCards(args.at(0), nick);
            else
                sendMessage("Impossible de trouver ""\x02" + args.at(0) + ", " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick + "\x0F"));
        }
    }
    else if (cmd == "cartes")
    {
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, ""\x02" + nick + "\x0F");
        else
            sendMessage("Il reste " + QString::number(cards->size()) + " cartes à piocher");
    }
    else if (cmd == "j")
    {
        Player *curr = players->getPlayer(currPlayer);
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, ""\x02" + nick + "\x0F");
        else if (currPlayer != nick)
            sendMessage("C'est au tour de " + curr->getColoredName());
        else if (args.size() < 2)
            sendMessage("\"!aide j\" pour savoir utiliser !j");
        else if (curr->getDeck()->contains(new Card(args.at(0), args.at(1))) || curr->getDeck()->contains(new Card("N", args.at(1))))
        {
            QString color = QString(args.at(0)).toUpper(), id = QString(args.at(1)).toUpper();
            if (lastCard->getColor() == color || lastCard->getId() == id || lastCard->getColor() == "N" || id == "J" || id == "+4")
            {
                if (color == "R" || color == "V" || color == "B" || color == "J")
                {
                    if (id == "J")
                    {
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard("N", id);
                        end = true;
                    }
                    else if (id == "+4")
                    {
                        if (cards->size() >= 4)
                        {
                            if (curr->getDeck()->containsId((lastCard->getId() == "+4" ? "" : lastCard->getId())) || curr->getDeck()->containsColor(lastCard->getColor()))
                                sendMessage("Vous ne pouvez pas jouer un " + Card("N", "+4").toString() + " si vous pouvez jouer une autre carte, " + curr->getColoredName());
                            else
                            {
                                lastCard = new Card(color, id);
                                curr->getDeck()->remCard("N", id);
                                Player *next = players->getPlayer(nextPlayer());
                                sendMessage(next->getColoredName() + " pioche 4 cartes");
                                sendCommand(IrcCommand::createNotice(next->getName(), next->getDeck()->randCards(4)));
                                next->cantPlay();
                                end = true;
                            }
                        }
                        else
                        {
                            lastCard = new Card(color, id);
                            curr->getDeck()->remCard("N", id);
                            Player *next = players->getPlayer(nextPlayer());
                            sendMessage(next->getColoredName() + " pioche " + QString::number(cards->size()) + " cartes");
                            sendCommand(IrcCommand::createNotice(next->getName(), next->getDeck()->randCards(cards->size())));
                            end = true;
                        }
                    }
                    else if (id == "+2")
                    {
                        if (cards->size() >= 2)
                        {
                            Player *next = players->getPlayer(nextPlayer());
                            sendMessage(next->getColoredName() + " pioche 2 cartes");
                            sendCommand(IrcCommand::createNotice(next->getName(), next->getDeck()->randCards(2)));
                            next->cantPlay();
                            lastCard = new Card(color, id);
                            curr->getDeck()->remCard(color, id);
                            end = true;
                        }
                        else
                        {
                            lastCard = new Card(color, id);
                            curr->getDeck()->remCard("N", id);
                            Player *next = players->getPlayer(nextPlayer());
                            sendMessage(next->getColoredName() + " pioche " + QString::number(cards->size()) + " cartes");
                            sendCommand(IrcCommand::createNotice(next->getName(), next->getDeck()->randCards(cards->size())));
                            end = true;
                        }
                    }
                    else if (id == "I")
                    {
                        if (players->size() == 2)
                            players->getPlayer(nextPlayer())->cantPlay();
                        else
                        {
                            sendMessage("Le sens de jeu est ""\x16""inversé");
                            inversed = !inversed;
                        }
                        lastCard = new Card(color, id);
                        curr->getDeck()->remCard(color, id);
                        end = true;
                    }
                    else if (id == "P")
                    {
                        players->getPlayer(nextPlayer())->cantPlay();
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
                    sendMessage("Cette couleur n'existe pas, " + curr->getColoredName());
            }
            else
                sendMessage("Vous ne pouvez pas jouer cette carte, " + curr->getColoredName());
        }
        else
        {
            sendMessage("Vous n'avez pas cette carte ou elle n'existe pas, " + curr->getColoredName());
        }
    }

    if (end && cards->isEmpty())
    {
        sendMessage("La pioche est vide");
        Player *winner = players->getPlayer(currPlayer);
        int minCards = 100;
        foreach (Player *w, players->getList())
        {
            if (w->getDeck()->size() < minCards)
            {
                minCards = w->getDeck()->size();
                winner = w;
            }
        }
        sendMessage(winner->getColoredName() + " a gagné !");
        clear();
        return;
    }
    else if (end && players->getPlayer(currPlayer)->getDeck()->size() == 1)
        sendMessage(players->getPlayer(currPlayer)->getColoredName() + " ""\x16""est en ""\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14""\x16"" !");
    else if (end && players->getPlayer(currPlayer)->getDeck()->size() == 0)
    {
        sendMessage(players->getPlayer(currPlayer)->getColoredName() + " a gagné !");
        clear();
        return;
    }

    if (end)
    {
        currPlayer = nextPlayer();
        drawed = false;
        if (!players->getPlayer(currPlayer)->canPlay())
        {
            sendMessage(players->getPlayer(currPlayer)->getColoredName() + " passe son tour");
            sendCommand(IrcCommand::createMessage(chan, " "));
            sendMessage("Carte visible : " + lastCard->toString());
            currPlayer = nextPlayer();
            sendMessage("C'est au tour de " + players->getPlayer(currPlayer)->getColoredName());
            showCards();
        }
        else
        {
            sendCommand(IrcCommand::createMessage(chan, " "));
            sendMessage("Carte visible : " + lastCard->toString());
            sendMessage("C'est au tour de " + players->getPlayer(currPlayer)->getColoredName());
            showCards();
        }
    }
}

bool UNO::isOp(QString user)
{
    return network()->prefixToMode(modes.value(user)) == "o" || network()->prefixToMode(modes.value(user)) == "q" ? true : false;
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
    return cards;
}
