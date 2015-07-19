#include <QCoreApplication>
#include "echos.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Echos echos(&a);
    return a.exec();
}
