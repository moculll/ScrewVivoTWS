#pragma once
#include <QtWidgets/QWidget>
#include <QTimer>
#include "VivoController/VivoController.h"
#include "MTrayMenu/MTrayMenu.h"
#include "nlohmann/json.hpp"
class ScrewVivoTWS
{
public:
    
    ScrewVivoTWS();
    ~ScrewVivoTWS();
    std::unique_ptr<VivoDevice> device;
    std::unique_ptr<MTrayMenu> trayMenu;

    std::shared_ptr<nlohmann::json> uiTextJsonData;
    std::unique_ptr<QTimer> readAsyncTimer;
private:
    bool loadJson(std::string fileName);
};
