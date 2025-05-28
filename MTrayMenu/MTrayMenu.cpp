#include "MTrayMenu/MTrayMenu.h"
#include <QApplication>
#include <QAccessible>
#include "spdlog/spdlog.h"
QPixmap MTrayMenu::loadCroppedIcon(const QString& path) {
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

void MTrayMenu::addOption(const QString &name, std::function<void()> callback)
{
    QAction* action = new QAction(name, trayMenu);

    if(callback)
        QObject::connect(action, &QAction::triggered, this, std::move(callback));

    QList<QAction*> actions = trayMenu->actions();
    if (!actions.isEmpty()) {
        trayMenu->insertAction(actions.first(), action);
    }
    else {
        trayMenu->addAction(action);
    }
}
QAction* findActionByText(QMenu* menu, const QString& name) {
    for (QAction* action : menu->actions()) {
        if (action->text() == name)
            return action;

        if (QMenu* subMenu = action->menu()) {
            QAction* result = findActionByText(subMenu, name);
            if (result)
                return result;
        }
    }
    return nullptr;
}
void MTrayMenu::addOption(const QString& name, const QString& parentName, std::function<void()> callback)
{
    QAction* parentAction = findActionByText(trayMenu, parentName);

    if (!parentAction) {
        spdlog::debug("can't find action: {}", parentName.toLocal8Bit().toStdString());
        return;
    }

    QMenu* subMenu = parentAction->menu();
    if (!subMenu) {
        subMenu = new QMenu(trayMenu);
        subMenu->setStyleSheet(MenuStyle);
        parentAction->setMenu(subMenu);
    }

    QAction* childAction = new QAction(name, trayMenu);
    if (callback)
        QObject::connect(childAction, &QAction::triggered, std::move(callback));
    subMenu->addAction(childAction);
}

void MTrayMenu::insertSeparator(const QString &name)
{
    QList<QAction*> actions = trayMenu->actions();
    for (int i = 0; i < actions.size(); ++i) {
        if (actions[i]->text() == name) {
            /* insert after */
            if (i + 1 < actions.size()) {
                trayMenu->insertSeparator(actions[i + 1]);
            }
            else {
                /* if it's the last one, insert in the end */
                trayMenu->addSeparator();
            }
            return;
        }
    }
}

void MTrayMenu::setTrayTitle(const QString &title)
{
    trayIcon->setToolTip(title);

}

QAction* MTrayMenu::findActionByObjectName(QMenu* menu, const QString& objName) {
    for (QAction* action : menu->actions()) {
        if (action->objectName() == objName)
            return action;

        if (QMenu* subMenu = action->menu()) {
            QAction* result = findActionByObjectName(subMenu, objName);
            if (result)
                return result;
        }
    }
    return nullptr;
}

MTrayMenu::MTrayMenu(QObject* parent) : QObject(parent)
{
    QAccessible::queryAccessibleInterface(this);
    trayIcon = new QSystemTrayIcon(this);
    trayMenu = new QMenu(nullptr);
    QPixmap pixmap = loadCroppedIcon(QCoreApplication::applicationDirPath() + "/bm.png");
    trayIcon->setIcon(QIcon(pixmap));
    
    
    QAction* exitAction = new QAction("退出", trayMenu);
    exitAction->setObjectName()

    QObject::connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    /* TOBE FIXED: display error if there is only one action... */
    /*
    QAction* showAction = new QAction("显示主界面", trayMenu);
    trayMenu->addSeparator();
    trayMenu->addAction(showAction);
    */
    trayMenu->addSeparator();
    trayMenu->addAction(exitAction);

    trayMenu->setStyleSheet(MenuStyle);
    /* avoid memory leak */
    trayIcon->setContextMenu(trayMenu);
    
}

MTrayMenu::~MTrayMenu()
{

}
