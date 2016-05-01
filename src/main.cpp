#include "uno.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QSettings* settings = new QSettings(qApp->applicationDirPath() + "/UNObox/settings.ini", QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
    QString lang = settings->value("language", "en").toString();
    QTranslator translator;
    translator.load("translations/" + lang);
    a.installTranslator(&translator);

    if (a.arguments().contains("--version") || a.arguments().contains("-v"))
    {
        qDebug() << qPrintable(QString("UNO [Update ") + COMMITDATE + "]");
        return 0;
    }

    UNO uno(&a);
    return a.exec();
}
