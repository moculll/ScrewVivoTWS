#pragma once
#include <QtWidgets/QWidget>
#include <QTimer>
#include "VivoController/VivoController.h"
#include "MTrayMenu/MTrayMenu.h"
#include "nlohmann/json.hpp"
#include <string>
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
    void getJsonToSettingStorage(nlohmann::json& data);
    void parseData(const std::vector<uint8_t>& buffer);
    struct VivoSettingStorage {
        VivoDeviceCommand::NoiseMaker::NoiseMode noiseMode;
        VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode deepxEffectMode;
        VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode doubleClickModeLeft;
        VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode doubleClickModeRight;
        VivoDeviceCommand::EarLongPressMaker::EarLongPressMode longPressModeLeft;
        VivoDeviceCommand::EarLongPressMaker::EarLongPressMode longPressModeRight;
        VivoDeviceCommand::AcceptCallMaker::DoubleClickMode doubleClickAcceptCallMode;
        VivoDeviceCommand::AcceptCallMaker::LongPressMode longPressRefuseCallMode;
        VivoDeviceCommand::WearDetectionMaker::WearDetectionMode wearDetectionMode;

        QString leftBattery;
        QString rightBattery;
    };
    VivoSettingStorage settingStorage;
};
