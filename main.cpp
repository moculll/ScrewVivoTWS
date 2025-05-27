#include "ScrewVivoTWS.h"
#include "MTrayMenu/MTrayMenu.h"
#include <cstdint>
#include <future>
#include <QtConcurrent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QObject>
#include <QCoreApplication>
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    auto screwVivoTWS = std::make_unique<ScrewVivoTWS>();
    
    return a.exec();
}
