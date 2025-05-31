#include "ScrewVivoTWS.h"
#include "Accessibility/AccessMgr.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <QTimer>
#include <iostream>
#include <string>
#include <ranges>
#include <algorithm>
using json = nlohmann::json;

bool ScrewVivoTWS::loadJson(std::string fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        spdlog::debug("can not load %s.", fileName);
        return false;
    }

    try {
        file >> *uiTextJsonData.get();
    }
    catch (json::parse_error& err) {
        spdlog::debug("%s parsing error: %s", fileName, err.what());
        return false;
    }
    return true;
}

void ScrewVivoTWS::getJsonToSettingStorage(nlohmann::json& data)
{
    QString eventId = QString::fromStdString(data["eventId"]);
    if(eventId != "A102|10084")
        return;
    QString batteryOrigin = QString::fromStdString(data["C"]);
    QStringList parts = batteryOrigin.split('_');
    settingStorage.leftBattery = parts.value(0);
    settingStorage.rightBattery = parts.value(1);


    QString wearEnable = QString::fromStdString(data["params"]["wear_enable"]);
    settingStorage.wearDetectionMode = static_cast<decltype(settingStorage.wearDetectionMode)>(wearEnable.toUInt());
    

    QString noiseMode = QString::fromStdString(data["params"]["noise"]);
    settingStorage.noiseMode = static_cast<decltype(settingStorage.noiseMode)>(noiseMode.toUInt());

    /* didn't figure out why vivo make read val different from write val, but it works */
    const static std::map<uint8_t, VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode> transMap = {
        {0x04, VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode},
        {0x06, VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PrevMode},
        {0x07, VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode},
        {0x03, VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::WakeupVoiceAssistantMode},
        {0xff, VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NoneMode},
    };

    QString doubleClickLeftMode = QString::fromStdString(data["params"]["double_click_l"]);

    settingStorage.doubleClickModeLeft.mode = static_cast<decltype(settingStorage.doubleClickModeLeft.mode)>(transMap.find(doubleClickLeftMode.toUInt())->second);

    QString doubleClickRightMode = QString::fromStdString(data["params"]["double_click_r"]);
    settingStorage.doubleClickModeRight.mode = static_cast<decltype(settingStorage.doubleClickModeRight.mode)>(transMap.find(doubleClickRightMode.toUInt())->second);

    QString longPressModeLeft = QString::fromStdString(data["params"]["long_click_l"]);
    settingStorage.longPressModeLeft.mode = static_cast<decltype(settingStorage.longPressModeLeft.mode)>(longPressModeLeft.toUInt());

    QString longPressModeRight = QString::fromStdString(data["params"]["long_click_r"]);
    settingStorage.longPressModeRight.mode = static_cast<decltype(settingStorage.longPressModeRight.mode)>(longPressModeRight.toUInt());

    QString deepxEffectMode = QString::fromStdString(data["params"]["eq_type"]);
    settingStorage.deepxEffectMode = static_cast<decltype(settingStorage.deepxEffectMode)>(deepxEffectMode.toUInt());

    /* ui set */
    json& textData = (*uiTextJsonData)[language];
    auto noiseModeAction = trayMenu->findActionByObjectName(settingStorage.toString(settingStorage.noiseMode));
    if (noiseModeAction) {
        noiseModeAction->setChecked(true);
    }

    auto doubleClickLeftModeAction = trayMenu->findActionByObjectName(settingStorage.doubleClickToString<decltype(settingStorage.doubleClickModeLeft.mode), true>(settingStorage.doubleClickModeLeft.mode));
    if (doubleClickLeftModeAction) {
        doubleClickLeftModeAction->setChecked(true);
    }
    
    auto doubleClickRightModeAction = trayMenu->findActionByObjectName(settingStorage.doubleClickToString<decltype(settingStorage.doubleClickModeRight.mode), false>(settingStorage.doubleClickModeRight.mode));
    
    if (doubleClickRightModeAction) {
        doubleClickRightModeAction->setChecked(true);
    }
    
    auto res = settingStorage.doubleClickToString<decltype(settingStorage.doubleClickModeRight.mode), false>(settingStorage.doubleClickModeRight.mode);
    spdlog::info("get result: {}", res.toLocal8Bit().toStdString());

    auto longPressModeLeftAction = trayMenu->findActionByObjectName(settingStorage.doubleClickToString<decltype(settingStorage.longPressModeLeft.mode), true>(settingStorage.longPressModeLeft.mode));
    if (longPressModeLeftAction) {
        longPressModeLeftAction->setChecked(true);
    }

    auto longPressModeRightAction = trayMenu->findActionByObjectName(settingStorage.doubleClickToString<decltype(settingStorage.longPressModeRight.mode), false>(settingStorage.longPressModeRight.mode));
    if (longPressModeRightAction) {
        longPressModeRightAction->setChecked(true);
    }

    auto deepxEffectModeAction = trayMenu->findActionByObjectName(settingStorage.toString(settingStorage.deepxEffectMode));
    if (deepxEffectModeAction) {
        deepxEffectModeAction->setChecked(true);
    }


}

void ScrewVivoTWS::parseData(const std::vector<uint8_t>& buffer) {
    std::string data(buffer.begin(), buffer.end());
    
    size_t start = data.find('{');
    size_t end = data.rfind('}');

    if (start == std::string::npos || end == std::string::npos || end <= start) {
        spdlog::warn("No valid JSON found in data.");
        return;
    }

    std::string json_str = data.substr(start, end - start + 1);

    try {
        nlohmann::json parsed = nlohmann::json::parse(json_str);
        getJsonToSettingStorage(parsed);
        
        spdlog::info("Parsed JSON: {}", parsed.dump(4)); // pretty print
    }
    catch (const nlohmann::json::parse_error& e) {
        spdlog::error("JSON parse error: {}", e.what());
    }
}

/* TOBE FIXED: status refresh & record */
ScrewVivoTWS::ScrewVivoTWS()
{
    std::unique_ptr accessMgr = std::make_unique<Accessibility::AccessMgr>(Accessibility::SpeakerType::QT_SAPI);
    accessMgr->speaker->init();
    
    trayMenu = std::make_unique<MTrayMenu>();
    trayMenu->setTrayTitle("Vivo APP Killer");

    uiTextJsonData = std::make_shared<nlohmann::json>();

    language = "Chinese";

    auto loadJsonResult = loadJson("UIText.json");
    if (loadJsonResult) {
        /* TOBE FIXED: language switch */
        json& textData = (*uiTextJsonData)[language];

        auto NoiseOption = QString::fromStdString(textData["Noise"]["title"]);
        auto TransparentModeOption = QString::fromStdString(textData["Noise"]["options"]["Transparent"]);
        auto ClostModeOption = QString::fromStdString(textData["Noise"]["options"]["Close"]);
        auto NoiseModeOption = QString::fromStdString(textData["Noise"]["options"]["Noise"]);

        settingStorage.addMapping(VivoDeviceCommand::NoiseMaker::NoiseMode::TransparentMode, TransparentModeOption);
        settingStorage.addMapping(VivoDeviceCommand::NoiseMaker::NoiseMode::CloseMode, ClostModeOption);
        settingStorage.addMapping(VivoDeviceCommand::NoiseMaker::NoiseMode::NoiseMode, NoiseModeOption);
        /*spdlog::info("get map: {}", settingStorage.toString(VivoDeviceCommand::NoiseMaker::NoiseMode::TransparentMode).toLocal8Bit().toStdString());*/
        trayMenu->addOption(NoiseOption, NoiseOption, nullptr);
        trayMenu->addOption(TransparentModeOption, TransparentModeOption, NoiseOption, [&]() {
            settingStorage.noiseMode = VivoDeviceCommand::NoiseMaker::NoiseMode::TransparentMode;
            VivoDeviceCommand::NoiseMaker noiseMaker(settingStorage.noiseMode);
            if (device)
                device->write(noiseMaker.data);
        }, true);

        trayMenu->addOption(ClostModeOption, ClostModeOption, NoiseOption, [&]() {
            settingStorage.noiseMode = VivoDeviceCommand::NoiseMaker::NoiseMode::CloseMode;
            VivoDeviceCommand::NoiseMaker noiseMaker(settingStorage.noiseMode);
            if (device)
                device->write(noiseMaker.data);
        }, true);

        trayMenu->addOption(NoiseModeOption, NoiseModeOption, NoiseOption, [&]() {
            settingStorage.noiseMode = VivoDeviceCommand::NoiseMaker::NoiseMode::NoiseMode;
            VivoDeviceCommand::NoiseMaker noiseMaker(settingStorage.noiseMode);
            if (device)
                device->write(noiseMaker.data);
        }, true);

        auto DeepXOption = QString::fromStdString(textData["DeepX"]["title"]);
        auto DeepXDefaultOption = QString::fromStdString(textData["DeepX"]["options"]["Default"]);
        auto DeepXHumanSoundOption = QString::fromStdString(textData["DeepX"]["options"]["HumanSound"]);
        auto DeepXDeepBassOption = QString::fromStdString(textData["DeepX"]["options"]["DeepBass"]);
        auto DeepXClearTrebleOption = QString::fromStdString(textData["DeepX"]["options"]["ClearTreble"]);
        auto DeepXSoothingOption = QString::fromStdString(textData["DeepX"]["options"]["Soothing"]);
        settingStorage.addMapping(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::DefaultMode, DeepXDefaultOption);
        settingStorage.addMapping(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::HumanSoundMode, DeepXHumanSoundOption);
        settingStorage.addMapping(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::DeepBassMode, DeepXDeepBassOption);
        settingStorage.addMapping(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::ClearTrebleMode, DeepXClearTrebleOption);
        settingStorage.addMapping(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::SoothingMode, DeepXSoothingOption);
        trayMenu->addOption(DeepXOption, DeepXOption, nullptr);
        trayMenu->addOption(DeepXDefaultOption, DeepXDefaultOption, DeepXOption, [&]() {
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::DefaultMode;
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
        }, true);

        trayMenu->addOption(DeepXHumanSoundOption, DeepXHumanSoundOption, DeepXOption, [&]() {
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::HumanSoundMode;
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
        }, true);

        trayMenu->addOption(DeepXDeepBassOption, DeepXDeepBassOption, DeepXOption, [&]() {
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::DeepBassMode;
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
        }, true);

        trayMenu->addOption(DeepXClearTrebleOption, DeepXClearTrebleOption, DeepXOption, [&]() {
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::ClearTrebleMode;
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
        }, true);

        trayMenu->addOption(DeepXSoothingOption, DeepXSoothingOption, DeepXOption, [&]() {
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::SoothingMode;
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
        }, true);
        
        auto DoubleClickRightOption = QString::fromStdString(textData["DoubleClickRight"]["title"]);
        auto DoubleClickRightPlayPauseOption = QString::fromStdString(textData["DoubleClickRight"]["options"]["PlayPause"]);
        auto DoubleClickRightPreviousOption = QString::fromStdString(textData["DoubleClickRight"]["options"]["Previous"]);
        auto DoubleClickRightNextOption = QString::fromStdString(textData["DoubleClickRight"]["options"]["Next"]);
        auto DoubleClickRightWakeUpAssistantOption = QString::fromStdString(textData["DoubleClickRight"]["options"]["WakeUpAssistant"]);
        auto DoubleClickRightNoneOption = QString::fromStdString(textData["DoubleClickRight"]["options"]["None"]);
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, false>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode, DoubleClickRightPlayPauseOption + "_double_r");
        
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, false>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PrevMode, DoubleClickRightPreviousOption + "_double_r");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, false>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode, DoubleClickRightNextOption + "_double_r");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, false>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::WakeupVoiceAssistantMode, DoubleClickRightWakeUpAssistantOption + "_double_r");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, false>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NoneMode, DoubleClickRightNoneOption + "_double_r");
        trayMenu->addOption(DoubleClickRightOption, DoubleClickRightOption, nullptr);
        trayMenu->addOption(DoubleClickRightPlayPauseOption, DoubleClickRightPlayPauseOption + "_double_r", DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeRight.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode;
            earDoubleClickMaker.setRightMode(settingStorage.doubleClickModeRight.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        trayMenu->addOption(DoubleClickRightPreviousOption, DoubleClickRightPreviousOption + "_double_r", DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeRight.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PrevMode;
            earDoubleClickMaker.setRightMode(settingStorage.doubleClickModeRight.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        trayMenu->addOption(DoubleClickRightNextOption, DoubleClickRightNextOption + "_double_r", DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeRight.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode;
            earDoubleClickMaker.setRightMode(settingStorage.doubleClickModeRight.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        trayMenu->addOption(DoubleClickRightWakeUpAssistantOption, DoubleClickRightWakeUpAssistantOption + "_double_r", DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeRight.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::WakeupVoiceAssistantMode;
            earDoubleClickMaker.setRightMode(settingStorage.doubleClickModeRight.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        trayMenu->addOption(DoubleClickRightNoneOption, DoubleClickRightNoneOption + "_double_r", DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeRight.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NoneMode;
            earDoubleClickMaker.setRightMode(settingStorage.doubleClickModeRight.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        auto DoubleClickLeftOption = QString::fromStdString(textData["DoubleClickLeft"]["title"]);
        auto DoubleClickLeftPlayPauseOption = QString::fromStdString(textData["DoubleClickLeft"]["options"]["PlayPause"]);
        auto DoubleClickLeftPreviousOption = QString::fromStdString(textData["DoubleClickLeft"]["options"]["Previous"]);
        auto DoubleClickLeftNextOption = QString::fromStdString(textData["DoubleClickLeft"]["options"]["Next"]);
        auto DoubleClickLeftWakeUpAssistantOption = QString::fromStdString(textData["DoubleClickLeft"]["options"]["WakeUpAssistant"]);
        auto DoubleClickLeftNoneOption = QString::fromStdString(textData["DoubleClickLeft"]["options"]["None"]);
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, true>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode, DoubleClickLeftPlayPauseOption + "_double_l");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, true>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PrevMode, DoubleClickLeftPreviousOption + "_double_l");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, true>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode, DoubleClickLeftNextOption + "_double_l");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, true>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::WakeupVoiceAssistantMode, DoubleClickLeftWakeUpAssistantOption + "_double_l");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, true>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NoneMode, DoubleClickLeftNoneOption + "_double_l");
        trayMenu->addOption(DoubleClickLeftOption, DoubleClickLeftOption, nullptr);
        trayMenu->addOption(DoubleClickLeftPlayPauseOption, DoubleClickLeftPlayPauseOption + "_double_l", DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeLeft.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode;
            earDoubleClickMaker.setLeftMode(settingStorage.doubleClickModeLeft.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        /*auto res1 = settingStorage.doubleClickToString<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, false>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode);
        spdlog::info("get res: {}", res1.toLocal8Bit().toStdString());

        auto res2 = settingStorage.doubleClickToString<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, true>(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode);
        spdlog::info("get res: {}", res2.toLocal8Bit().toStdString());*/
        trayMenu->addOption(DoubleClickLeftPreviousOption, DoubleClickLeftPreviousOption + "_double_l", DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeLeft.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PrevMode;
            earDoubleClickMaker.setLeftMode(settingStorage.doubleClickModeLeft.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        trayMenu->addOption(DoubleClickLeftNextOption, DoubleClickLeftNextOption + "_double_l", DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeLeft.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode;
            earDoubleClickMaker.setLeftMode(settingStorage.doubleClickModeLeft.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        trayMenu->addOption(DoubleClickLeftWakeUpAssistantOption, DoubleClickLeftWakeUpAssistantOption + "_double_l", DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeLeft.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::WakeupVoiceAssistantMode;
            earDoubleClickMaker.setLeftMode(settingStorage.doubleClickModeLeft.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        trayMenu->addOption(DoubleClickLeftNoneOption, DoubleClickLeftNoneOption + "_double_l", DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            settingStorage.doubleClickModeLeft.mode = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NoneMode;
            earDoubleClickMaker.setLeftMode(settingStorage.doubleClickModeLeft.mode);
            if (device)
                device->write(earDoubleClickMaker.data);
        }, true);

        /* TOBE FIXED: status record */
        auto PressRightOption = QString::fromStdString(textData["PressRight"]["title"]);
        auto PressRightNoiseSwitchOption = QString::fromStdString(textData["PressRight"]["options"]["NoiseSwitch"]);
        auto PressRightTransparentSwitchOption = QString::fromStdString(textData["PressRight"]["options"]["TransparentSwitch"]);
        auto PressRightNoiseTransparentSwitchOption = QString::fromStdString(textData["PressRight"]["options"]["NoiseTransparentSwitch"]);
        auto PressRightNoiseCloseTransparentSwitchOption = QString::fromStdString(textData["PressRight"]["options"]["NoiseCloseTransparentSwitch"]);
        auto PressRightNoneOption = QString::fromStdString(textData["PressRight"]["options"]["None"]);
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, false>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseNoiseMode, PressRightNoiseSwitchOption + "_r");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, false>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseTransparentMode, PressRightTransparentSwitchOption + "_r");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, false>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode, PressRightNoiseTransparentSwitchOption + "_r");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, false>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndTransparentMode, PressRightNoiseCloseTransparentSwitchOption + "_r");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, false>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::NoneMode, PressRightNoneOption + "_long_r");
        trayMenu->addOption(PressRightOption, PressRightOption, nullptr);
        trayMenu->addOption(PressRightNoiseSwitchOption, PressRightNoiseSwitchOption + "_r", PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeRight.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseNoiseMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);

        trayMenu->addOption(PressRightTransparentSwitchOption, PressRightTransparentSwitchOption + "_r", PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeRight.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseTransparentMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);

        trayMenu->addOption(PressRightNoiseTransparentSwitchOption, PressRightNoiseTransparentSwitchOption + "_r", PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeRight.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndTransparentMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);

        trayMenu->addOption(PressRightNoiseCloseTransparentSwitchOption, PressRightNoiseCloseTransparentSwitchOption + "_r", PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeRight.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);

        trayMenu->addOption(PressRightNoneOption, PressRightNoneOption + "_long_r", PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeRight.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::NoneMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);


        /* TOBE FIXED: status record */
        auto PressLeftOption = QString::fromStdString(textData["PressLeft"]["title"]);
        auto PressLeftNoiseSwitchOption = QString::fromStdString(textData["PressLeft"]["options"]["NoiseSwitch"]);
        auto PressLeftTransparentSwitchOption = QString::fromStdString(textData["PressLeft"]["options"]["TransparentSwitch"]);
        auto PressLeftNoiseTransparentSwitchOption = QString::fromStdString(textData["PressLeft"]["options"]["NoiseTransparentSwitch"]);
        auto PressLeftNoiseCloseTransparentSwitchOption = QString::fromStdString(textData["PressLeft"]["options"]["NoiseCloseTransparentSwitch"]);
        auto PressLeftNoneOption = QString::fromStdString(textData["PressLeft"]["options"]["None"]);
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, true>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseNoiseMode, PressLeftNoiseSwitchOption + "_l");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, true>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseTransparentMode, PressLeftTransparentSwitchOption + "_l");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, true>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode, PressLeftNoiseTransparentSwitchOption + "_l");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, true>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndTransparentMode, PressLeftNoiseCloseTransparentSwitchOption + "_l");
        settingStorage.addDoubleClickMapping<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, true>(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::NoneMode, PressLeftNoneOption + "_long_l");
        trayMenu->addOption(PressLeftOption, PressLeftOption, nullptr);
        trayMenu->addOption(PressLeftNoiseSwitchOption, PressLeftNoiseSwitchOption + "_l", PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeLeft.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseNoiseMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);

        trayMenu->addOption(PressLeftTransparentSwitchOption, PressLeftTransparentSwitchOption + "_l", PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeLeft.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseTransparentMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);

        trayMenu->addOption(PressLeftNoiseTransparentSwitchOption, PressLeftNoiseTransparentSwitchOption + "_l", PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeLeft.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndTransparentMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);

        trayMenu->addOption(PressLeftNoiseCloseTransparentSwitchOption, PressLeftNoiseCloseTransparentSwitchOption + "_l", PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeLeft.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);

        trayMenu->addOption(PressLeftNoneOption, PressLeftNoneOption + "_long_l", PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            settingStorage.longPressModeLeft.mode = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::NoneMode;
            earLongPressMaker.setMode(settingStorage.longPressModeLeft.mode, settingStorage.longPressModeRight.mode);
            if (device)
                device->write(earLongPressMaker.data);
        }, true);
        /*auto actionl = trayMenu->findActionByObjectName(PressLeftNoneOption + "_l");
        if (actionl) {
            spdlog::info("foundl: {}", actionl->objectName().toLocal8Bit().toStdString());
        }

        auto actionr = trayMenu->findActionByObjectName(PressLeftNoneOption + "_r");
        if (actionr) {
            spdlog::info("foundl: {}", actionr->objectName().toLocal8Bit().toStdString());
        }*/
        auto doubleClickAccepCallOption = QString::fromStdString(textData["DoubleClickAcceptCall"]["title"]);
        
        trayMenu->addOption(doubleClickAccepCallOption, doubleClickAccepCallOption, [&]() {
            VivoDeviceCommand::AcceptCallMaker acceptCallMaker;
            if (settingStorage.doubleClickAcceptCallMode == VivoDeviceCommand::AcceptCallMaker::DoubleClickMode::DoubleClickNoneMode) {
                settingStorage.doubleClickAcceptCallMode = VivoDeviceCommand::AcceptCallMaker::DoubleClickMode::DoubleClickAcceprOrRefuseMode;
            }
            else {
                settingStorage.doubleClickAcceptCallMode = VivoDeviceCommand::AcceptCallMaker::DoubleClickMode::DoubleClickNoneMode;
            }
            acceptCallMaker.setMode(settingStorage.doubleClickAcceptCallMode, settingStorage.longPressRefuseCallMode);
            if (device)
                device->write(acceptCallMaker.data);
        }, true);

        auto pressRefuseCallOption = QString::fromStdString(textData["PressRefuseCall"]["title"]);
        trayMenu->addOption(pressRefuseCallOption, pressRefuseCallOption, [&]() {
            VivoDeviceCommand::AcceptCallMaker acceptCallMaker;
            if (settingStorage.longPressRefuseCallMode == VivoDeviceCommand::AcceptCallMaker::LongPressMode::LongPressNoneMode) {
                settingStorage.longPressRefuseCallMode = VivoDeviceCommand::AcceptCallMaker::LongPressMode::LongPressRefuseMode;
            }
            else {
                settingStorage.longPressRefuseCallMode = VivoDeviceCommand::AcceptCallMaker::LongPressMode::LongPressNoneMode;
            }
            acceptCallMaker.setMode(settingStorage.doubleClickAcceptCallMode, settingStorage.longPressRefuseCallMode);
            if (device)
                device->write(acceptCallMaker.data);
        }, true);

        auto wearDetionOption = QString::fromStdString(textData["WearDetection"]["title"]);
        trayMenu->addOption(wearDetionOption, wearDetionOption, [&]() {
            if (settingStorage.wearDetectionMode == VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OnMode) {
                settingStorage.wearDetectionMode = VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OffMode;
            }
            else {
                settingStorage.wearDetectionMode = VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OnMode;
            }
            VivoDeviceCommand::WearDetectionMaker wearDetectionMaker(settingStorage.wearDetectionMode);
            if (device)
                device->write(wearDetectionMaker.data);
        }, true);

        QString BatteryTitle = QString::fromStdString(textData["Battery"]["title"]);
        trayMenu->addOption(BatteryTitle, BatteryTitle, nullptr);

        trayMenu->show();
    }
    /* TOBE FIXED: default initialization */
    else {

    }
    /*
    {
    "A": "1", -> acceptCallMaker
    "B": "2_1",
    "C": "92_89", -> battery
    "D": "1748145525",
    "P": "4_4",
    "V": "2.5.5_2.5.5",
    "eventId": "A102|10084",
    "params": {
        "A": "1_0",
        "B": "2_1",
        "P": "4_4",
        "RSSI": "-68_-41_-59",
        "double_click_l": "255",
        "double_click_r": "255",
        "eq_type": "2",
        "long_click_l": "255",
        "long_click_r": "255",
        "noise": "1",
        "wear_enable": "1"
    }
}
    
    */
    /* qtimer runs in qt main thread, so we can use bluetooth device in it */
    readAsyncTimer = std::make_unique<QTimer>();
    QObject::connect(readAsyncTimer.get(), &QTimer::timeout, [&]() {
        if (!device->isConnected())
            return;
        auto result = device->read();
        
        
        parseData(result);
        
    });


    /*trayMenu->addOption("测试", "测试", [&]() {
        VivoDeviceCommand::EarDoubleClickMaker wearDetectionMaker;
        wearDetectionMaker.setLeftMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode);
        wearDetectionMaker.data.data()[wearDetectionMaker.data.size() - 1] = 0x01;
        if (device)
            device->write(wearDetectionMaker.data);
    });*/

    readAsyncTimer->start(1000);

   
    /*menu->insertSeparator("test");*/
    

    /* init bluetooth device in the end, in case ui haven't been loaded */
    device = VivoController::init();
    if (device)
        device->connect();

}

ScrewVivoTWS::~ScrewVivoTWS()
{}
