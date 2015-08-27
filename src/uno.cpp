#include "uno.h"

UNO::UNO(QCoreApplication *_parent) : IrcConnection(_parent)
{
    qsrand(QTime::currentTime().msec());

    cards = new Cards;
    players = new Players;
    users = new Users;
    lastCard = new Card("", "");
    inGame = false, preGame = false, drawed = false, inversed = false, inPing = false, inVersion = false;

    settings = new QSettings(qApp->applicationDirPath() + "/ini/settings.ini", QSettings::IniFormat);
    settings->setIniCodec("UTF-8");

    slaps = new QSettings(qApp->applicationDirPath() + "/ini/slaps.ini", QSettings::IniFormat);
    slaps->setIniCodec("UTF-8");

    colors = new QSettings(qApp->applicationDirPath() + "/ini/colors.ini", QSettings::IniFormat);
    colors->setIniCodec("UTF-8");

    scores = new QSettings(qApp->applicationDirPath() + "/ini/scores.ini", QSettings::IniFormat);
    scores->setIniCodec("UTF-8");

    qputenv("IRC_DEBUG", settings->value("debug", "0").toByteArray());

    setServers(QStringList(settings->value("server", "irc.t411.io").toString()));
    chan = (settings->value("chan", "#uno").toString().startsWith("#") ? "" : "#") + settings->value("chan", "#uno").toString();
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
    colors->sync();
    scores->sync();
    delete cards;
    delete players;
    delete users;
    delete lastCard;
    delete settings;
    delete slaps;
    delete colors;
    delete scores;
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
    sendCommand(IrcCommand::createNames(chan));
    sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + message->nick()));
    if (preGame)
        sendMessage("Envie de jouer au UNO ? Tapez ""\x02""!rejoindre""\x0F"" pour rejoindre la partie en préparation !");
    else if (!inGame)
        sendMessage("Envie de jouer au UNO ? Tapez ""\x02""!aide""\x0F"" pour afficher la liste des commandes");
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
            sendCommand(IrcCommand::createMessage(chan, "\x03""01,03" + green + "\x03""01,04" + red + "\x0F""\x02"" " + users->get(currPing)->getColoredName() + ": " + QString::number(pingTime) + "ms"));
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

void UNO::preGameTimeout()
{
    if (!preGame)
        return;

    sendMessage("Délai de préparation de la partie dépassé, la partie est annulée");
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
        sendMessage(players->get(nick)->getColoredName() + " a quitté la partie");
        players->remove(nick);
        turns.removeOne(nick);

        if (players->size() == 1 && inGame)
        {
            sendMessage(players->get(players->first())->getColoredName() + " a gagné la partie !");
            clear();
        }
        else if (nick == currPlayer && inGame)
        {
            currPlayer = nextPlayer();
            sendMessage("C'est donc au tour de " + players->get(currPlayer)->getColoredName());
        }
        else if (players->size() == 0 && preGame)
        {
            sendMessage("Plus aucun joueur, la préparation de la partie est annulée");
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
    delete cards;
    cards = new Cards();
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
        else if (cmd == "update")
        {
            sendMessage("Mise à jour de " + nickName());
            QProcess::startDetached(qApp->applicationDirPath() + "/updateUNO");
            qApp->exit();
        }
        else if (cmd == "sendraw")
            sendRaw(args.join(" "));
        else if (cmd == "kick" && !args.isEmpty())
            remPlayer(args.first());
    }
    else if (cmd == "op")
        sendCommand(IrcCommand::createMessage("NickServ", "STATUS " + nick));

    if (cmd == "color") {
        if (args.isEmpty())
        {
            sendMessage("Utilisation : !color numéro");
            sendMessage("Couleurs disponibles : ""\x03""022  ""\x03""033  ""\x03""044  ""\x03""055  ""\x03""066  ""\x03""077  ""\x03""088  ""\x03""099  ""\x03""1010  ""\x03""1111  ""\x03""1212  ""\x03""1313");
            sendMessage("Couleur actuelle : " + users->get(nick)->getColoredName());
        }
        else if (args.first().toInt() >= 2 && args.first().toInt() <= 13 && args.size() == 1)
        {
            if (players->contains(nick) && (inGame || preGame))
                sendMessage("Vous ne pouvez pas changer votre couleur en partie ou en préparation, " + players->get(nick)->getColoredName());
            else
            {
                users->get(nick)->setColor(args.first().toShort());
                colors->setValue(nick, args.first().toShort());
                sendMessage("Vous avez changé votre couleur, " + users->get(nick)->getColoredName());
            }
        }
        else if (args.size() == 1)
            sendMessage(args.first() + " n'est pas une couleur valide");
        else
            sendMessage("Utilisation : !color numéro");
    }
    else if (cmd == "nocolor") {
        users->get(nick)->setColored(!users->get(nick)->getColored());
        if (users->get(nick)->getColored())
            sendNotice(nick, "Affichage des couleurs activé");
        else
            sendNotice(nick, "Affichage des couleurs désactivé");
    }
    else if (cmd == "scores")
    {
        showScores();
    }
    else if (cmd == "aide" || cmd == "help")
    {
        if (args.isEmpty())
        {
            sendMessage("\x02""Tout le temps :""\x0F"" !version, !regles, !color, !nocolor, !scores");
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
            sendMessage("Exemple : pour jouer un %c il faut écrire !j v 7", new Card("V", "7"));
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
            sendMessage("\x02""Aide :""\x0F"" !cartes : affiche le nombre de cartes restantes dans la pioche et dans les mains des joueurs");
        else if (args.first() == "regles" || args.first() == "rules")
            sendMessage("\x02""Aide :""\x0F"" !regles : affiche les règles du jeu");
        else if (args.first() == "ping")
            sendMessage("\x02""Aide :""\x0F"" !ping : affiche la latence entre ""\x02" + nickName() + "\x0F"" et vous");
        else if (args.first() == "version")
            sendMessage("\x02""Aide :""\x0F"" !version : affiche la version du client de l'utilisateur donné");
        else if (args.first() == "slaps")
            command(nickName(), "slaps", QStringList() << nick);
        else if (args.first() == "color")
            sendMessage("\x02""Aide :""\x0F"" !color : permet de choisir la couleur de son pseudonyme");
        else if (args.first() == "nocolor")
            sendMessage("\x02""Aide :""\x0F"" !nocolor : active/désactive l'affichage des couleurs des cartes");
        else if (args.first() == "scores")
            sendMessage("\x02""Aide :""\x0F"" !scores : affiche le classement des joueurs");
        else
            sendMessage("Cette commande n'existe pas, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "regles" || cmd == "rules")
        sendMessage("\x02""Règles :""\x0F"" http://tuxange.org/unorules/");
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
            sendCommand(IrcCommand::createMessage(chan, "\x02" + args.first() + "\x0F"" n'a pas été trouvé"));
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
            sendCommand(IrcCommand::createMessage(chan, "Un ping est déjà en cours, " + (players->contains(nick) ? players->get(nick)->getColoredName() : "\x02" + nick)));
    }
    else if (cmd == "liste" || cmd == "list")
    {
        if (inGame || preGame)
            sendMessage("Dans la partie: " + players->list());
        else
            sendMessage("Il n'y a pas de partie en cours, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "quitter")
    {
        if (players->contains(nick))
            remPlayer(nick);
        else if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, " + users->get(nick)->getColoredName());
        else
            sendMessage("Vous n'êtes pas dans cette partie, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "rejoindre")
    {
        if (preGame)
        {
            if (!players->contains(nick))
            {
                Player *p = new Player(nick, this);
                players->add(p);
                turns.insert(qrand() % turns.size(), nick);
                // turns.append(nick);
                sendMessage(p->getColoredName() + " a rejoint la partie");
                sendMessage("Il y a " + QString::number(players->size()) + " joueurs dans la partie");
            }
            else
                sendMessage("Vous êtes déjà dans la partie, " + players->get(nick)->getColoredName());
        }
        else if (inGame)
            sendMessage("Impossible de rejoindre une partie en cours de jeu, " + users->get(nick)->getColoredName());
        else
            sendMessage("Il n'y a pas de partie en préparation, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "commencer")
    {
        if (!players->contains(nick) && preGame)
            sendMessage("Vous n'êtes pas dans cette partie, " + users->get(nick)->getColoredName());
        else if (players->size() > 1 && preGame)
        {
            int rand;
            do { rand = qrand() % cards->size() - 1; } while (cards->get(rand)->getId() == "+4");
            lastCard = cards->get(rand);
            cards->remove(rand);
            foreach (Player *w, players->getList())
                w->getDeck()->init();
            sendMessage(" --- ");
            sendMessage("Carte visible : %c", lastCard);
            currPlayer = players->rand()->getName();

            if (lastCard->getId() == "+2")
            {
                sendMessage(players->get(currPlayer)->getColoredName() + " pioche 2 cartes");
                sendNotice(currPlayer, players->get(currPlayer)->getDeck()->randCards(2, users->get(currPlayer)->getColored()));
                sendMessage(players->get(currPlayer)->getColoredName() + " passe son tour");
                currPlayer = nextPlayer();
            }
            else if (lastCard->getId() == "I" && players->size() > 2)
            {
                sendMessage("Le sens du jeu est ""\x16""inversé");
                inversed = true;
            }
            else if (lastCard->getId() == "P" || lastCard->getId() == "I")
            {
                sendMessage(players->get(currPlayer)->getColoredName() + " passe son tour");
                currPlayer = nextPlayer();
            }

            sendMessage("C'est au tour de " + players->get(currPlayer)->getColoredName());

            foreach (Player *w, players->getList())
                showCards(w->getName());

            inGame = true;
            preGame = false;
        }
        else if (preGame)
            sendMessage("Il n'y a pas assez de joueurs pour commencer la partie, " + players->get(nick)->getColoredName());
        else if (inGame)
            sendMessage("Une partie est déjà en cours, " + users->get(nick)->getColoredName());
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
            QTimer::singleShot(60*5*1000, this, SLOT(preGameTimeout()));
        }
        else if (inGame)
            sendMessage("Une partie est déjà en cours, " + users->get(nick)->getColoredName());
        else // preGame
            sendMessage("Une partie est déjà en préparation, " + users->get(nick)->getColoredName());
    }
    else if (cmd == "pioche")
    {
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, " + users->get(nick)->getColoredName());
        else if (!drawed && currPlayer == nick)
        {
            sendNotice(currPlayer, players->get(currPlayer)->getDeck()->randCards(1));
            sendMessage("Carte visible : %c", lastCard);
            drawed = true;
        }
        else if (currPlayer == nick)
            sendMessage("Vous avez déjà pioché, " + players->get(nick)->getColoredName());
        else
            sendMessage("C'est au tour de " + players->get(currPlayer)->getColoredName());
    }
    else if (cmd == "fin" || cmd == "f")
    {
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, " + users->get(nick)->getColoredName());
        else if (drawed && currPlayer == nick)
            end = true;
        else if (currPlayer == nick)
            sendMessage("Vous n'avez pas pioché, " + players->get(currPlayer)->getColoredName());
        else
            sendMessage("C'est au tour de " + players->get(currPlayer)->getColoredName());
    }
    else if (cmd == "main")
    {
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, " + users->get(nick)->getColoredName());
        else if (!isOp(nick) && !args.isEmpty())
            sendMessage("Vous n'avez pas le droit de voir les cartes des autres joueurs, " + users->get(nick)->getColoredName());
        else if (args.isEmpty())
        {
            sendMessage("Carte visible : %c", lastCard);
            sendMessage("C'est au tour de " + players->get(currPlayer)->getColoredName());
            if (players->contains(nick)) showCards(nick);
        }
        else if (isOp(nick))
        {
            if (players->contains(args.at(0)))
                showCards(args.at(0), nick);
            else
                sendMessage("Impossible de trouver ""\x02" + args.at(0) + ", " + users->get(nick)->getColoredName());
            flushMessages();
        }
    }
    else if (cmd == "cartes")
    {
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, " + users->get(nick)->getColoredName());
        else
        {
            sendMessage("Il reste " + QString::number(cards->size()) + " cartes à piocher");
            foreach (Player *w, players->getList())
                sendMessage(w->getColoredName() + " a " + QString::number(w->getDeck()->size()) + " cartes");
        }
    }
    else if (cmd == "j")
    {
        Player *curr = players->get(currPlayer);
        if (!inGame)
            sendMessage("Il n'y a pas de partie en cours, " + users->get(nick)->getColoredName());
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
                            lastCard = new Card(color, id);
                            curr->getDeck()->remCard("N", id);
                            Player *next = players->get(nextPlayer());
                            sendMessage(next->getColoredName() + " pioche 4 cartes");
                            sendNotice(next->getName(), next->getDeck()->randCards(4, users->get(next->getName())->getColored()));
                            next->cantPlay();
                            end = true;
                        }
                        else
                        {
                            lastCard = new Card(color, id);
                            curr->getDeck()->remCard("N", id);
                            Player *next = players->get(nextPlayer());
                            sendMessage(next->getColoredName() + " pioche " + QString::number(cards->size()) + " cartes");
                            sendNotice(next->getName(), next->getDeck()->randCards(cards->size(), users->get(next->getName())->getColored()));
                            end = true;
                        }
                    }
                    else if (id == "+2")
                    {
                        if (cards->size() >= 2)
                        {
                            Player *next = players->get(nextPlayer());
                            sendMessage(next->getColoredName() + " pioche 2 cartes");
                            sendNotice(next->getName(), next->getDeck()->randCards(2, users->get(next->getName())->getColored()));
                            next->cantPlay();
                            lastCard = new Card(color, id);
                            curr->getDeck()->remCard(color, id);
                            end = true;
                        }
                        else
                        {
                            lastCard = new Card(color, id);
                            curr->getDeck()->remCard("N", id);
                            Player *next = players->get(nextPlayer());
                            sendMessage(next->getColoredName() + " pioche " + QString::number(cards->size()) + " cartes");
                            sendNotice(next->getName(), next->getDeck()->randCards(cards->size(), users->get(next->getName())->getColored()));
                            end = true;
                        }
                    }
                    else if (id == "I")
                    {
                        if (players->size() == 2)
                            players->get(nextPlayer())->cantPlay();
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
        Player *winner = players->get(currPlayer);
        int minCards = 100;
        foreach (Player *w, players->getList())
        {
            if (w->getDeck()->size() < minCards)
            {
                minCards = w->getDeck()->size();
                winner = w;
            }
        }
        sendMessage(winner->getColoredName() + " a gagné la partie !");

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
        sendMessage(players->get(currPlayer)->getColoredName() + " a gagné " + QString::number(points - origpoints) + " points !");

        sendMessage(" --- ");
        showScores();
        flushMessages();
        clear();
        return;
    }
    else if (end && players->get(currPlayer)->getDeck()->size() == 1)
        sendMessage(players->get(currPlayer)->getColoredName() + " ""\x16""est en ""\x03""01,15[""\x02""\x03""04,15UNO""\x0F""\x03""01,15]""\x02""\x03""00,14""\x16"" !");
    else if (end && players->get(currPlayer)->getDeck()->size() == 0)
    {
        sendMessage(players->get(currPlayer)->getColoredName() + " a gagné la partie !");

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
        sendMessage(players->get(currPlayer)->getColoredName() + " a gagné " + QString::number(points - origpoints) + " points !");

        sendMessage(" --- ");
        showScores();
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
            sendMessage(players->get(currPlayer)->getColoredName() + " passe son tour");
            sendMessage(" --- ");
            sendMessage("Carte visible : %c", lastCard);
            currPlayer = nextPlayer();
            sendMessage("C'est au tour de " + players->get(currPlayer)->getColoredName());
            showCards();
        }
        else
        {
            sendMessage(" --- ");
            sendMessage("Carte visible : %c", lastCard);
            sendMessage("C'est au tour de " + players->get(currPlayer)->getColoredName());
            showCards();
        }
    }

    flushMessages();
}

void UNO::showScores()
{
    if (scores->allKeys().isEmpty())
    {
        sendMessage("Aucune partie n'a été jouée, le classement commencera à la fin de la première partie");
        return;
    }

    sendMessage("Classement :");
    QStringList people = scores->allKeys();

    foreach (QString w, people)
        if (w.startsWith("Total/") || w.startsWith("Points/"))
            people.removeOne(w);

    QString curr;
    int ratio = 0;
    int currratio;

    for (int i = 0; i < 10; i++)
    {
        curr = "";
        ratio = 0;
        foreach (QString w, people)
        {
            currratio = (scores->value("Points/" + w).toInt() / scores->value(w).toInt()) * ((scores->value(w).toInt() * 10 / scores->value("Total/" + w).toInt())) / 10;
            if (currratio > ratio)
            {
                curr = w;
                ratio = currratio;
            }
        }
        sendMessage(QString::number(i + 1) + ". " + (users->contains(curr) ? users->get(curr)->getColoredName() : "\x02" + curr + "\x0F") + " : " + QString::number(ratio) + " (" + scores->value("Points/" + curr).toString() + " points sur " + scores->value(curr).toString() + " victoire" + (scores->value(curr).toInt() > 1 ? "s" : "") + " pour " + scores->value("Total/" + curr).toString() + " partie" + (scores->value("Total/" + curr).toInt() > 1 ? "s" : "") + " jouée" + (scores->value("Total/" + curr).toInt() > 1 ? "s" : "") + ")");
        people.removeOne(curr);

        if (people.isEmpty())
            break;
    }

    flushMessages();
}

bool UNO::isOp(QString user)
{
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
    return cards;
}

Users* UNO::getUsers() const
{
    return users;
}
