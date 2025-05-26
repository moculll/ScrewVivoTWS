#include "ScrewVivoTWS.h"
#include <QtConcurrent>
#include <QPushButton>
#include "Accessibility/AccessMgr.h"
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
QPixmap loadCroppedIcon(const QString& path) {
    QPixmap original(path);
    QImage image = original.toImage();

    int top = image.height(), bottom = 0, left = image.width(), right = 0;

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) > 10) {
                top = qMin(top, y);
                bottom = qMax(bottom, y);
                left = qMin(left, x);
                right = qMax(right, x);
            }
        }
    }

    if (top >= bottom || left >= right) {
        return original;
    }

    QRect cropRect(left, top, right - left + 1, bottom - top + 1);
    QPixmap cropped = original.copy(cropRect);
    return cropped.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

ScrewVivoTWS::ScrewVivoTWS(QWidget *parent)
    : QWidget(parent)
{
    /*ui.setupUi(this);*/
    
    QPixmap pixmap = loadCroppedIcon(QCoreApplication::applicationDirPath() + "/bm.png");
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(pixmap));
    trayIcon->setToolTip("ScrewVivoTWS");
    QMenu* trayMenu = new QMenu(this);
    QAction* showAction = new QAction("显示主界面", this);
    QAction* exitAction = new QAction("退出", this);
    connect(showAction, &QAction::triggered, this, &QWidget::showNormal);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    trayMenu->addAction(showAction);
    trayMenu->addSeparator();
    trayMenu->addAction(exitAction);
    QString MenuStyle = R"(
        QMenu {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            padding: 1px 0px;
            font: 8.65pt 'Segoe UI';
            color: #000000;
            margin: 0px;
            min-width: 120px;
        }

        QMenu::item {
            padding: 2px 24px;
            margin: 0px;
            height: 18px;
            background-color: transparent;
            border: none;
        }

        QMenu::item:selected {
            background-color: #00c8d7;
            color: #000000;
        }

        QMenu::separator {
            height: 1px;
            background: #e0e0e0;
            margin: 2px 0px;
        }

        QMenu::icon {
            width: 0px;
            height: 0px;
        }
    )";
    trayMenu->setStyleSheet(MenuStyle);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
 

    /*std::unique_ptr accessMgr = std::make_unique<Accessibility::AccessMgr>(Accessibility::SpeakerType::QT_SAPI);
    accessMgr->speaker->init();
    accessMgr->speaker->speak(L"this is a test.", true);*/
    device = VivoController::init();
    if(device)
        device->connect();

    connect(ui.sendBtn, &QPushButton::clicked, [&]() {
        
        VivoDeviceCommand::WearDetectionMaker doubleClick;
        doubleClick.setMode(VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OffMode);

        if(device)
            device->write(doubleClick.data);
        
        
    });
    
    
    
    
}

ScrewVivoTWS::~ScrewVivoTWS()
{}
