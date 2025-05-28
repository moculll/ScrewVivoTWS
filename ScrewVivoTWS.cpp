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

    QString doubleClickLeftMode = QString::fromStdString(data["params"]["double_click_l"]);
    settingStorage.doubleClickModeLeft = static_cast<decltype(settingStorage.doubleClickModeLeft)>(doubleClickLeftMode.toUInt());

    QString doubleClickRightMode = QString::fromStdString(data["params"]["double_click_r"]);
    settingStorage.doubleClickModeRight = static_cast<decltype(settingStorage.doubleClickModeRight)>(doubleClickRightMode.toUInt());

    QString longPressModeLeft = QString::fromStdString(data["params"]["long_click_l"]);
    settingStorage.longPressModeLeft = static_cast<decltype(settingStorage.longPressModeLeft)>(longPressModeLeft.toUInt());

    QString longPressModeRight = QString::fromStdString(data["params"]["long_click_r"]);
    settingStorage.longPressModeRight = static_cast<decltype(settingStorage.longPressModeRight)>(longPressModeRight.toUInt());

    QString deepxEffectMode = QString::fromStdString(data["params"]["eq_type"]);
    settingStorage.deepxEffectMode = static_cast<decltype(settingStorage.deepxEffectMode)>(deepxEffectMode.toUInt());


    


}

void ScrewVivoTWS::parseData(const std::vector<uint8_t>& buffer) {
    // 将 vector 转为 string
    std::string data(buffer.begin(), buffer.end());

    // 找到第一个 '{' 和最后一个 '}'
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

    auto loadJsonResult = loadJson("UIText.json");
    if (loadJsonResult) {
        /* TOBE FIXED: language switch */
        json& textData = (*uiTextJsonData)["Chinese"];

        auto NoiseOption = QString::fromStdString(textData["Noise"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["Noise"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["Noise"]["options"]["Transparent"]), NoiseOption, [&]() {
            settingStorage.noiseMode = VivoDeviceCommand::NoiseMaker::NoiseMode::TransparentMode;

            VivoDeviceCommand::NoiseMaker noiseMaker(settingStorage.noiseMode);

            if (device)
                device->write(noiseMaker.data); 
        });

        trayMenu->addOption(QString::fromStdString(textData["Noise"]["options"]["Close"]), NoiseOption, [&]() {
            settingStorage.noiseMode = VivoDeviceCommand::NoiseMaker::NoiseMode::CloseMode;

            VivoDeviceCommand::NoiseMaker noiseMaker(settingStorage.noiseMode);

            if (device)
                device->write(noiseMaker.data);
            
        });
        trayMenu->addOption(QString::fromStdString(textData["Noise"]["options"]["Noise"]), NoiseOption, [&]() {
            settingStorage.noiseMode = VivoDeviceCommand::NoiseMaker::NoiseMode::NoiseMode;

            VivoDeviceCommand::NoiseMaker noiseMaker(settingStorage.noiseMode);

            if (device)
                device->write(noiseMaker.data);
        });

        auto DeepXOption = QString::fromStdString(textData["DeepX"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["Default"]), DeepXOption, [&]() {
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::DefaultMode;

            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["HumanSound"]), DeepXOption, [&]() {
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::HumanSoundMode;

            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["DeepBass"]), DeepXOption, [&]() {
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::DeepBassMode;

            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["ClearTreble"]), DeepXOption, [&]() {
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::ClearTrebleMode;

            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["Soothing"]), DeepXOption, [&]() {
            
            settingStorage.deepxEffectMode = VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::SoothingMode;

            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(settingStorage.deepxEffectMode);
            if (device)
                device->write(depxEffectMaker.data);
        });
        
        
        auto DoubleClickRightOption = QString::fromStdString(textData["DoubleClickRight"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["PlayPause"]), DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;

            settingStorage.doubleClickModeRight = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode;


            earDoubleClickMaker.setRightMode(settingStorage.doubleClickModeRight);
            if (device)
                device->write(earDoubleClickMaker.data);
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["Previous"]), DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;

            settingStorage.doubleClickModeRight = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PrevMode;


            earDoubleClickMaker.setRightMode(settingStorage.doubleClickModeRight);
            if (device)
                device->write(earDoubleClickMaker.data);
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["Next"]), DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;

            settingStorage.doubleClickModeRight = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode;


            earDoubleClickMaker.setRightMode(settingStorage.doubleClickModeRight);
            if (device)
                device->write(earDoubleClickMaker.data);
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["WakeUpAssistant"]), DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;

            settingStorage.doubleClickModeRight = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode;


            earDoubleClickMaker.setRightMode(settingStorage.doubleClickModeRight);
            if (device)
                device->write(earDoubleClickMaker.data);
            
        });

        auto DoubleClickLeftOption = QString::fromStdString(textData["DoubleClickLeft"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["PlayPause"]), DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;

            settingStorage.doubleClickModeLeft = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode;


            earDoubleClickMaker.setLeftMode(settingStorage.doubleClickModeLeft);
            if (device)
                device->write(earDoubleClickMaker.data);
            

        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["Previous"]), DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;

            settingStorage.doubleClickModeLeft = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PrevMode;


            earDoubleClickMaker.setLeftMode(settingStorage.doubleClickModeLeft);
            if (device)
                device->write(earDoubleClickMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["Next"]), DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;

            settingStorage.doubleClickModeLeft = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode;


            earDoubleClickMaker.setLeftMode(settingStorage.doubleClickModeLeft);
            if (device)
                device->write(earDoubleClickMaker.data);
           
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["WakeUpAssistant"]), DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;

            settingStorage.doubleClickModeLeft = VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::WakeupVoiceAssistantMode;


            earDoubleClickMaker.setLeftMode(settingStorage.doubleClickModeLeft);
            if (device)
                device->write(earDoubleClickMaker.data);

        });
        
        /* TOBE FIXED: status record */
        auto PressRightOption = QString::fromStdString(textData["PressRight"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["NoiseSwitch"]), PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeRight = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseNoiseMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);

        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["TransparentSwitch"]), PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeRight = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseTransparentMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);

        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["NoiseTransparentSwitch"]), PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeRight = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndTransparentMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);

        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["NoiseCloseTransparentSwitch"]), PressRightOption, [&]() {

            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeRight = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);

        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["None"]), PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeRight = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::NoneMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);

        });

        /* TOBE FIXED: status record */
        auto PressLeftOption = QString::fromStdString(textData["PressLeft"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["NoiseSwitch"]), PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeLeft = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseNoiseMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);

        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["TransparentSwitch"]), PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeLeft = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseTransparentMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);

        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["NoiseTransparentSwitch"]), PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeLeft = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndTransparentMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);

        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["NoiseCloseTransparentSwitch"]), PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeLeft = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["None"]), PressLeftOption, [&]() {

            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;

            settingStorage.longPressModeLeft = VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::NoneMode;


            earLongPressMaker.setMode(settingStorage.longPressModeLeft, settingStorage.longPressModeRight);
            if (device)
                device->write(earLongPressMaker.data);
        });
        
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickAcceptCall"]["title"]), [&]() {
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
        });
        
        trayMenu->addOption(QString::fromStdString(textData["PressRefuseCall"]["title"]), [&]() {
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
        });

        
        trayMenu->addOption(QString::fromStdString(textData["WearDetection"]["title"]), [&]() {

            if (settingStorage.wearDetectionMode == VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OnMode) {
                settingStorage.wearDetectionMode = VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OffMode;
            }
            else {
                settingStorage.wearDetectionMode = VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OnMode;
            }

            VivoDeviceCommand::WearDetectionMaker wearDetectionMaker(settingStorage.wearDetectionMode);
            
            if (device)
                device->write(wearDetectionMaker.data);
        });

        QString BatteryTitle = QString::fromStdString(textData["Battery"]["title"]);
        trayMenu->addOption(BatteryTitle, nullptr);
        
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
        /*std::vector<uint8_t> pattern = { 0xFF, 0x03, 0x00, 0x03, 0x00, 0x1b, 0x81 };
        auto it = std::search(result.begin(), result.end(), pattern.begin(), pattern.end());

        if (it != result.end()) {
            spdlog::info("contains 81!!!!!");
        }*/
        
        parseData(result);
        
    });


    trayMenu->addOption("测试", [&]() {
        VivoDeviceCommand::WearDetectionMaker wearDetectionMaker(VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OnMode);
        if (device)
            device->write(wearDetectionMaker.data);
    });

    readAsyncTimer->start(1000);

   
    /*menu->insertSeparator("test");*/
    

    /* init bluetooth device in the end, in case ui haven't been loaded */
    device = VivoController::init();
    if (device)
        device->connect();

}

ScrewVivoTWS::~ScrewVivoTWS()
{}
