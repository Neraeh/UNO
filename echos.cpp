#include "echos.h"

Echos::Echos(QCoreApplication *parent) : IrcConnection()
{
    cards = new Cards;
    //qputenv("IRC_DEBUG", "1");
    //qsrand(QTime::currentTime().msec());

    setServers(QStringList("irc.t411.io"));
    setUserName("EchosTest");
    setNickName("EchosTest");
    setRealName("Echos");
    sendCommand(IrcCommand::createJoin("#bottest"));
    sendCommand(IrcCommand::createMessage("#bottest", "Hello"));
    open();

    QObject::connect(this, SIGNAL(disconnected()), parent, SLOT(quit()));
}

Cards* Echos::getCards() const
{
    return cards;
}
