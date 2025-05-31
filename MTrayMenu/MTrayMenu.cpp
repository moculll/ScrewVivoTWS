#include "MTrayMenu/MTrayMenu.h"
#include <QApplication>
#include <QAccessible>
#include <QActionGroup>
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

void MTrayMenu::addOption(const QString& text, const QString& objectName, std::function<void()> callback, bool checkable) {
    QAction* action = new QAction(text, trayMenu);
    action->setCheckable(checkable);
    action->setObjectName(objectName);

    if (callback)
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

void MTrayMenu::setActionText(const QString& objName, const QString& newText) {
    QAction* action = findActionByObjectName(trayMenu, objName);
    if (action) {
        action->setText(newText);
    }
}
void MTrayMenu::addOption(const QString& text,const QString& objectName, const QString& parentObjName, std::function<void()> callback, bool checkable) {
    QAction* parentAction = findActionByObjectName(trayMenu, parentObjName);

    if (!parentAction) {
        spdlog::debug("Can't find action: {}", parentObjName.toLocal8Bit().toStdString());
        return;
    }
    QAction* childAction = new QAction(text, trayMenu);
    childAction->setObjectName(objectName);
    childAction->setCheckable(checkable);

    QMenu* subMenu = parentAction->menu();

    QActionGroup* actionGroup = nullptr;
    if (!subMenu) {
        subMenu = new QMenu(trayMenu);
        subMenu->setStyleSheet(MenuStyle);
        parentAction->setMenu(subMenu);
        actionGroup = new QActionGroup(subMenu);
        actionGroup->setExclusive(true);
        
    }
    else {
        actionGroup = subMenu->actions()[0]->actionGroup();

    }
    actionGroup->addAction(childAction);
    
    
    
    if (callback)
        QObject::connect(childAction, &QAction::triggered, std::move(callback));
    
    subMenu->addAction(childAction);
}


void MTrayMenu::insertSeparator(const QString& objName) {
    QAction* action = findActionByObjectName(trayMenu, objName);
    if (!action) return;

    QList<QAction*> actions = trayMenu->actions();
    int index = actions.indexOf(action);

    if (index != -1) {
        if (index + 1 < actions.size()) {
            trayMenu->insertSeparator(actions[index + 1]);
        }
        else {
            trayMenu->addSeparator();
        }
    }
}

void MTrayMenu::setTrayTitle(const QString &title)
{
    trayIcon->setToolTip(title);

}

void MTrayMenu::setChecked(const QString& objName, bool checked)
{
    auto action = findActionByObjectName(objName);
    if(action->isCheckable())
        action->setChecked(checked);
}

QAction* MTrayMenu::findActionByObjectName(QMenu *menu, const QString& objName) {
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

QAction* MTrayMenu::findActionByObjectName(const QString& objName) {
    for (QAction* action : trayMenu->actions()) {
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
    exitAction->setObjectName("退出");

    QObject::connect(exitAction, &QAction::triggered, this, &QApplication::quit);
    /* TOBE FIXED: display error if there is only one action... */
    
    /*QAction* showAction = new QAction("显示主界面", trayMenu);
    trayMenu->addSeparator();
    trayMenu->addAction(showAction);
    showAction->setCheckable(true);*/
    
    trayMenu->addSeparator();
    trayMenu->addAction(exitAction);

    trayMenu->setStyleSheet(MenuStyle);
    /* didn't set trayMenu's parent */
    trayIcon->setContextMenu(trayMenu);

    /* win+B -> enter no response fix */
    QObject::connect(trayIcon, &QSystemTrayIcon::activated, [&] (QSystemTrayIcon::ActivationReason reason) {
        if (QSystemTrayIcon::Trigger == reason) {
            QPoint pos = trayIcon->geometry().topLeft();
            trayMenu->popup(pos);
        }
        
    });
 
}

MTrayMenu::~MTrayMenu()
{
    delete trayMenu;
}
