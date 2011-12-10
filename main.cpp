#include <QtCore/QCoreApplication>
#include "helpers.h"
#include "artematimer.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    ArtemaTimer * t = new ArtemaTimer(0,2000,"Tpol");
    t->start();
    a.exec();
    return 0;
}
