#include "ScrewVivoTWS.h"
#include <cstdint>
#include <future>
#include <QtConcurrent>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ScrewVivoTWS w;
    
    
    w.show();
    return a.exec();
}
