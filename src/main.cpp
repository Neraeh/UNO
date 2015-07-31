#include "uno.h"

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QCoreApplication a(argc, argv);
    UNO uno(&a);
    return a.exec();
}
