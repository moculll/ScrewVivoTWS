#pragma once
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QObject>
#include <QAccessible>

class MTrayMenu : public QObject
{
	Q_OBJECT
public:
	QPixmap loadCroppedIcon(const QString& path);
    void addOption(const QString& name, std::function<void()> callback);
    void addOption(const QString& name, const QString& parentName, std::function<void()> callback);
    void insertSeparator(const QString& name);

    void setTrayTitle(const QString& title);

    void show()
    {
        
        trayIcon->show();
    }

    void hide()
    {
        trayIcon->hide();
    }

    explicit MTrayMenu(QObject* parent = nullptr);
	~MTrayMenu();

protected:
	QSystemTrayIcon* trayIcon;
	QMenu* trayMenu;
    inline static QString MenuStyle = R"(
        QMenu {
            background-color: #f1f1f1;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            padding: 1px 0px;
            font: 8.5pt 'Microsoft YaHei';
            color: #000000;
            margin: 0px;
            min-width: 110px;
        }

        QMenu::item {
            padding: 2px 24px;
            margin: 0px;
            height: 18px;
            background-color: transparent;
            border: none;
            min-width: 100px;
            min-height: 16px;
        }

        QMenu::item:selected {
            background-color: #91c7f6;
            color: #000000;
            padding: 2px 24px;
            margin: 0px;
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

        QMenu::right-arrow {
            subcontrol-origin: padding;
            subcontrol-position: right center;
            padding-right: 6px;
            width: 8px;
            height: 8px;
        }
    )";
};
