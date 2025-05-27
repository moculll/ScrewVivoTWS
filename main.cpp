#include "ScrewVivoTWS.h"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    auto screwVivoTWS = std::make_unique<ScrewVivoTWS>();
    
    return a.exec();
}
