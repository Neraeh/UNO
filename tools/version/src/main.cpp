#include <QCoreApplication>
#include <QTextCodec>
#include <QDebug>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    #if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    #else
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    #endif

    QCoreApplication a(argc, argv);
    
    QFile* file = new QFile(qApp->applicationDirPath() + 
    #ifdef Q_OS_WIN
        "/.."
    #endif
    "/../../../libcommuni/src/core/ircconnection.cpp");
    QTextStream content(file);
    content.setCodec("UTF-8");

    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Impossible de trouver libcommuni";
        return 1;
    }

    QString origContent = content.readAll();
    file->close();

    if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << "Impossible d'obtenir les droits d'écriture dans le dossier de libcommuni";
        return 1;
    }

    content << origContent.replace("reply = QLatin1String(\"VERSION libcommuni \") + Irc::version() + QLatin1String(\" - https://communi.github.io\");",
                                   "reply = QString(\"VERSION UNO par Shayy, basé sur la version Java de Feeling [libcommuni \") + Irc::version() + QString(\" - https://communi.github.io]\");");
    file->waitForBytesWritten(-1);
    file->close();
    delete file;

    return 0;
}
