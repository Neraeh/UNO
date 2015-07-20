#include <QCoreApplication>
#include "echos.h"

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QCoreApplication a(argc, argv);
    Echos echos(&a);
    return a.exec();
}
