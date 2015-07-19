#include "irc.h"

Irc::Irc() : QObject()
{
    socket = new QTcpSocket;
    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
    QObject::connect(socket, SIGNAL(connected()), this, SLOT(connectToServer()));
    QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectToServer(QAbstractSocket::SocketError)));
}

Irc::~Irc() {}

void Irc::connectToServer(QAbstractSocket::SocketError _error)
{
    if (_error == QAbstractSocket::UnknownSocketError)
    {
        switch(socket->state())
        {
            case QAbstractSocket::UnconnectedState: // Tentative de connexion
                emit networkOutput("Connecting to " + server + ":" + QString::number(port));
                emit debugOutput("Connexion au serveur " + server + ":" + QString::number(port) + " en cours");
                socket->connectToHost(server, port);
                break;
            case QAbstractSocket::ConnectedState: // Connecté
                emit networkOutput("Connected");
                emit debugOutput("Connecté");
                socket->write(QString("NICK " + nick + "\r\n").toUtf8());
                emit debugOutput("NICK " + nick);
                socket->write(QString("USER " + nick + " " + nick + " " + nick + " :" + nick + "\r\n").toUtf8());
                emit debugOutput("USER " + nick + " " + nick + " " + nick + " :" + nick);
                QTimer::singleShot(2000, this, SLOT(joinChans()));
                break;
            default:
                emit networkOutput("Unknown error");
                emit debugOutput("Erreur de connexion inconnue");
        }
    }
    else
    {
        emit networkOutput("Error: " + socket->errorString());
        emit debugOutput("Erreur de connexion");
        emit debugOutput(socket->errorString());
    }
}

void Irc::joinChans()
{
    foreach (Chan* w, chans) { socket->write(QString("JOIN " + w->getName() + "\r\n").toUtf8()); emit debugOutput("JOIN " + w->getName()); }
}

void Irc::readData()
{
    QString readLine = socket->readLine();
    readLine.chop(2);
    emit debugOutput(readLine);

    QStringList cmd = readLine.split(" ", QString::SkipEmptyParts);

    if (cmd.at(0) == "PING")
    {
        socket->write(QString("PONG :" + readLine.mid(6) + "\r\n").toUtf8());
        emit debugOutput("PONG :" + readLine.mid(6));
    }
    else if (cmd.at(1) == "PRIVMSG")
    {
        QString channel = QString(cmd.at(2)).startsWith(":") ? QString(cmd.at(2)).mid(1) : cmd.at(2);
        QString msg;
        for (int i = 3; i < cmd.size(); i++)
            msg.append(cmd.at(i) + " ");
        emit onMessage(QString(cmd.at(0)).left(QString(cmd.at(0)).indexOf("!")).mid(1), channel, msg.mid(1).trimmed());
    }
    else if (cmd.at(1) == "JOIN")
    {
        QString channel = QString(cmd.at(2)).startsWith(":") ? QString(cmd.at(2)).mid(1) : cmd.at(2);
        bool exists = false;
        foreach (Chan* w, chans) if (w->getName() == channel) exists = true;
        Chan *chan = new Chan;
        if (!exists) chans.append(new Chan(this, channel));
        foreach (Chan *w, chans) if (w->getName() == channel) chan = w;
        QString nick, realname, host;
        nick = QString(cmd.at(0)).left(QString(cmd.at(0)).indexOf("!")).mid(1);
        host = QString(cmd.at(0)).mid(QString(cmd.at(0)).indexOf("@") + 1);
        realname = QString(cmd.at(0)).mid(nick.length() + 2);
        realname.chop(host.length() + 1);
        chan->getUserlist().append(new User(nick, realname, host, ""));
        emit onJoin(nick, channel, host, realname);
    }
    else if (cmd.size() > 3)
    {
        if (cmd.at(3) == "=")
        {
            Chan *currChan = new Chan;
            foreach (Chan* w, chans) if (w->getName() == cmd.at(4)) currChan = w;
            QStringList users = cmd;
            users.removeFirst(); users.removeFirst(); users.removeFirst(); users.removeFirst(); users.removeFirst(); users.removeFirst();
            foreach (QString w, users) currChan->addUser(new User(w));
            emit usersChange(cmd.at(4), users);
        }
        else if (cmd.at(1) == QString::number(332))
        {
            Chan *currChan = new Chan;
            foreach (Chan* w, chans) if (w->getName() == cmd.at(3)) currChan = w;
            QString msg;
            for (int i = 4; i < cmd.size(); i++)
                msg.append(cmd.at(i) + " ");
            msg = msg.startsWith(":") ? msg.mid(1) : msg;
            currChan->setTopic(msg);
            emit topicChanged(cmd.at(3), msg);
        }
    }

    if (socket->canReadLine()) readData();
}

void Irc::disconnectFromServer()
{
    socket->write(QString("QUIT :" + nick + "\r\n").toUtf8());
    emit debugOutput("QUIT :" + nick);
    socket->flush();
    socket->disconnect();
    emit networkOutput("Disconnected");
}

void Irc::connect(const QString _nick, const QString _server, const unsigned long int _port, const QString _realname)
{
    nick = _nick; server = _server; port = _port; realname = _realname;
    this->connectToServer();
}

void Irc::disconnect()
{
    this->disconnectFromServer();
}

bool Irc::isConnected()
{
    return socket->state() == QAbstractSocket::ConnectedState ? true : false;
}

QString Irc::getNick()
{
    return nick;
}

QList<Chan*>* Irc::getChans()
{
    return &chans;
}

void Irc::join(QString _chan)
{
    socket->write(QString("JOIN " + _chan + "\r\n").toUtf8());
    emit debugOutput("JOIN " + _chan);
}

void Irc::part(QString _chan)
{
    socket->write(QString("PART " + _chan + "\r\n").toUtf8());
    emit debugOutput("PART " + _chan);
    foreach (Chan *w, chans) if (w->getName() == _chan) chans.removeAll(w);
}

void Irc::topic(QString _chan)
{
    socket->write(QString("TOPIC " + _chan + "\r\n").toUtf8());
}

void Irc::setNick(QString _nick)
{
    socket->write(QString("NICK :" + _nick + "\r\n").toUtf8());
    emit debugOutput("NICK :" + _nick);
    nick = _nick;
}

void Irc::sendMessage(QString _target, QString _message)
{
    socket->write(QString("PRIVMSG " + _target + " :" + _message + "\r\n").toUtf8());
    emit debugOutput("PRIVMSG " + _target + " :" + _message);
}

void Irc::sendNotice(QString _target, QString _notice)
{
    socket->write(QString("NOTICE " + _target + " :" + _notice + "\r\n").toUtf8());
    emit debugOutput("NOTICE " + _target + " :" + _notice);
}

void Irc::sendAction(QString _target, QString _action)
{
    socket->write(QString("PRIVMSG " + _target + " :\u0001ACTION " + _action + "\u0001\r\n").toUtf8());
    emit debugOutput("PRIVMSG " + _target + " :\u0001ACTION " + _action + "\u0001");
}
