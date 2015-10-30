#include "uno.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    if (a.arguments().contains("--version") || a.arguments().contains("-v"))
    {
        qDebug() << qPrintable(QString("UNO [Update ") + COMMITDATE + "]");
        return 0;
    }

    UNO uno(&a);
    return a.exec();
}
