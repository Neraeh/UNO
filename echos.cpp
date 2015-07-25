#include "echos.h"

Echos::Echos(QCoreApplication *_parent) : IrcConnection(_parent)
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
    QObject::connect(this, SIGNAL(modeMessageReceived(IrcModeMessage*)), this, SLOT(onMode(IrcModeMessage*)));
    QObject::connect(this, SIGNAL(namesMessageReceived(IrcNamesMessage*)), this, SLOT(onNames(IrcNamesMessage*)));
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
    else if (message->content().startsWith("!"))
    {
        QStringList args = message->content().split(" ", QString::SkipEmptyParts);
        args.removeFirst();
        command(message->nick(), message->content().split(" ", QString::SkipEmptyParts).first().mid(1), args);
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

void Echos::onMode(IrcModeMessage *message)
{
    if (message->kind() != IrcModeMessage::User)
        return;
    sendCommand(IrcCommand::createNames(chan));
}

void Echos::onNames(IrcNamesMessage *message)
{
    foreach (QString w, message->names())
        modes.insert(startsWithMode(w) ? w.mid(1) : w, startsWithMode(w) ? w.at(0) : QString());
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
        sendCommand(IrcCommand::createMode(chan, "+o", message->content().split(" ").at(3)));
    else if (message->content().startsWith("VERSION"))
        sendMessage("\x02" + message->nick() + "\x0F"" utilise:" + message->content().mid(message->content().indexOf(" ")));
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
            sendMessage("\x03""01,03" + green + "\x03""01,04" + red + "\x0F"" " + currPing + ": " + QString::number(pingTime) + "ms");
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
    modes.remove(message->nick());
}

void Echos::onQuit(IrcQuitMessage *message)
{
    remPlayer(message->nick());
    modes.remove(message->nick());
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
        sendMessage(players->getPlayer(nick)->getColoredName() + " a quitté la partie");

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
        else if (players->isEmpty() && preGame)
        {
            sendMessage("Plus aucun joueur, la préparation de la partie est annulée");
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

void Echos::sendMessage(QString message)
{
    message.replace("\x02", "\x03""04,15");
    message.replace("\x0F", "\x03""00,14");
    sendCommand(IrcCommand::createMessage(chan, "\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14 " + message + " "));
}

void Echos::command(QString nick, QString cmd, QStringList args)
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
            sendMessage("\x02""Tout le temps:""\x0F"" !ping, !version, !slaps, !regles");
            sendMessage("\x02""Avant et pendant la partie:""\x0F"" !liste, !quitter");
            sendMessage("\x02""Avant la partie:""\x0F"" !uno, !rejoindre, !commencer");
            sendMessage("\x02""Pendant la partie:""\x0F"" !j, !pioche, !fin, !main, !cartes");
            sendMessage("\x02""!aide commande""\x0F"" pour avoir des détails sur une commande (exemple: !aide rejoindre)");
        }
        else if (args.first() == "uno")
            sendMessage("\x02""Aide:""\x0F"" !uno : commencer la préparation d'une nouvelle partie");
        else if (args.first() == "j")
        {
            sendMessage("\x02""Aide:""\x0F"" !j");
            sendMessage("Utilisation : !j couleur carte");
            sendMessage("Exemple : pour jouer un " + Card("V", "7").toString() + " il faut écrire !j v 7");
        }
        else if (args.first() == "rejoindre")
            sendMessage("\x02""Aide:""\x0F"" !rejoindre : rejoindre la partie en préparation");
        else if (args.first() == "quitter")
            sendMessage("\x02""Aide:""\x0F"" !quitter : quitter la partie");
        else if (args.first() == "commencer")
            sendMessage("\x02""Aide:""\x0F"" !commencer : commencer la partie en préparation");
        else if (args.first() == "liste" || args.first() == "list")
            sendMessage("\x02""Aide:""\x0F"" !liste : affiche la liste des joueurs dans la partie");
        else if (args.first() == "pioche")
            sendMessage("\x02""Aide:""\x0F"" !pioche : piocher une carte");
        else if (args.first() == "fin" || args.first() == "f")
            sendMessage("\x02""Aide:""\x0F"" !fin : passer son tour");
        else if (args.first() == "main" || args.first() == "m")
            sendMessage("\x02""Aide:""\x0F"" !main : affiche les cartes dans votre main et la carte visible");
        else if (args.first() == "cartes")
            sendMessage("\x02""Aide:""\x0F"" !cartes : affiche le nombre de cartes restantes dans la pioche");
        else if (args.first() == "regles" || args.first() == "rules")
            sendMessage("\x02""Aide:""\x0F"" !regles : affiche les règles du jeu");
        else if (args.first() == "ping")
            sendMessage("\x02""Aide:""\x0F"" !ping : affiche la latence entre ""\x02" + nickName() + "\x0F"" et vous");
        else if (args.first() == "version")
            sendMessage("\x02""Aide:""\x0F"" !version : affiche la version du client de l'utilisateur donné");
        else if (args.first() == "slaps")
            command(nickName(), "slaps", QStringList() << nick);
        else
            sendMessage("Cette commande n'existe pas, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick));
    }
    else if (cmd == "regles" || cmd == "rules")
        sendMessage("\x02""Règles: ""\x0F""http://tuxange.org/unorules/");
    else if (cmd == "version")
    {
        if (args.isEmpty())
            sendCommand(IrcCommand::createVersion(nick));
        else if (modes.contains(args.first()))
            sendCommand(IrcCommand::createVersion(args.first()));
        else
            sendMessage("\x02" + args.first() + "\x0F"" n'a pas été trouvé");
    }
    else if (cmd == "slaps")
        sendMessage("Pas encore implémentée");
    else if (cmd == "ping")
    {
        if (currPing == "" || QTime::currentTime().msecsSinceStartOfDay() - pingTimeBegin > 20000)
        {
            pingTime = 1000000;
            pingCount = 0;
            inPing = true;
            currPing = nick;
            pingTimeBegin = QTime::currentTime().msecsSinceStartOfDay();
            sendCommand(IrcCommand::createCtcpRequest(nick, "PING " + QString::number(pingTimeBegin)));
        }
        else
            sendMessage("Un ping est déjà en cours, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick));
    }
    else if (cmd == "liste" || cmd == "list")
    {
        if (inGame || preGame)
            sendMessage("Dans la partie: " + players->list());
        else
            sendMessage("Il n'y a pas de partie en cours, ""\x02" + nick);
    }
    else if (cmd == "quitter")
    {
        remPlayer(nick);
    }
    else if (cmd == "rejoindre")
    {
        if (preGame)
        {
            if (!players->contains(nick))
            {
                Player *p = new Player(nick, this);
                players->add(p);
                sendMessage(p->getColoredName() + " a rejoint la partie");
                sendMessage("Il y a " + QString::number(players->size()) + " joueurs dans la partie");
            }
            else
                sendMessage("Vous êtes déjà dans la partie, " + players->getPlayer(nick)->getColoredName());
        }
        else if (inGame)
            sendMessage("Impossible de rejoindre une partie en cours de jeu, ""\x02" + nick);
        else
            sendMessage("Il n'y a pas de partie en préparation, ""\x02" + nick);
    }
    else if (cmd == "commencer")
    {
        if (!players->contains(nick) && preGame)
            sendMessage("Vous n'êtes pas dans cette partie, ""\x02" + nick);
        else if (players->size() > 1 && preGame)
        {
            QTime time = QTime::currentTime();
            qsrand((uint)time.msec());
            int rand;
            do { rand = qrand() % cards->size() - 1; } while (cards->get(rand) == new Card("N", "+4"));
            lastCard = cards->get(rand);
            cards->remove(rand);
            sendMessage("Carte visible : " + lastCard->toString());
            currPlayer = players->rand()->getName();

            if (lastCard->getId() == "+2")
            {
                sendCommand(IrcCommand::createNotice(currPlayer, players->getPlayer(currPlayer)->getDeck()->randCards(2)));
                sendMessage(players->getPlayer(currPlayer)->getName() + " passe son tour");
                currPlayer = nextPlayer();
            }
            else if (lastCard->getId() == "I")
            {
                sendMessage("Le sens de jeu est ""\x16""inversé");
                inversed = true;
            }
            else if (lastCard->getId() == "P")
            {
                sendMessage(players->getPlayer(currPlayer)->getName() + " passe son tour");
                currPlayer = nextPlayer();
            }

            sendMessage("C'est au tour de " + players->getPlayer(currPlayer)->getName());

            foreach (Player *w, players->getList())
                sendCommand(IrcCommand::createNotice(w->getName(), w->getDeck()->toString()));

            inGame = true;
        }
        else if (preGame)
            sendMessage("Il n'y a pas assez de joueurs pour commencer la partie, " + players->getPlayer(nick)->getColoredName());
        else if (inGame)
            sendMessage("Une partie est déjà en cours, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick));
        else
            sendMessage("Il n'y a pas de partie en préparation, ""\x02" + nick);
    }
    else if (cmd == "uno")
    {
        if (!inGame && !preGame)
        {
            preGame = true;
            Player *p = new Player(nick, this);
            players->add(p);
            sendMessage(p->getColoredName() + " a créé une nouvelle partie");
            sendMessage(p->getColoredName() + " a rejoint la partie");
            sendMessage("Il y a 1 joueur dans la partie");
        }
        else if (inGame)
            sendMessage("Une partie est déjà en cours, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick));
        else // preGame
            sendMessage("Une partie est déjà en préparation, " + (players->contains(nick) ? players->getPlayer(nick)->getColoredName() : "\x02" + nick));
    }
}

bool Echos::isOp(QString user)
{
    return network()->prefixToMode(modes.value(user)) == "o" || network()->prefixToMode(modes.value(user)) == "q" ? true : false;
}

bool Echos::startsWithMode(QString nick)
{
    QString w = nick.at(0);
    if (w == network()->modeToPrefix("q") || w == network()->modeToPrefix("a") || w == network()->modeToPrefix("o") || w == network()->modeToPrefix("h") || w == network()->modeToPrefix("v"))
        return true;
    return false;
}

Cards* Echos::getCards() const
{
    return cards;
}
