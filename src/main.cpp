#include "uno.h"

int main(int argc, char *argv[])
{
    #if (QT_VERSION <= QT_VERSION_CHECK(5, 0, 0))
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    #else
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    #endif
    QCoreApplication a(argc, argv);
    UNO uno(&a);
    return a.exec();
}
