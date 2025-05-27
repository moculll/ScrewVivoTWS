#include "ScrewVivoTWS.h"
#include "Accessibility/AccessMgr.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <QTimer>
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
void parseData(const std::vector<uint8_t>& buffer) {
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
            VivoDeviceCommand::NoiseMaker noiseMaker(VivoDeviceCommand::NoiseMaker::NoiseMode::TransparentMode);

            if (device)
                device->write(noiseMaker.data);
                
            
                
        });

        trayMenu->addOption(QString::fromStdString(textData["Noise"]["options"]["Close"]), NoiseOption, [&]() {
            VivoDeviceCommand::NoiseMaker noiseMaker(VivoDeviceCommand::NoiseMaker::NoiseMode::CloseMode);
          

            if (device)
                device->write(noiseMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["Noise"]["options"]["Noise"]), NoiseOption, [&]() {
            VivoDeviceCommand::NoiseMaker noiseMaker(VivoDeviceCommand::NoiseMaker::NoiseMode::NoiseMode);


            if (device)
                device->write(noiseMaker.data);
        });

        auto DeepXOption = QString::fromStdString(textData["DeepX"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["Default"]), DeepXOption, [&]() {
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::DefaultMode);
            if (device)
                device->write(depxEffectMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["HumanSound"]), DeepXOption, [&]() {
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::HumanSoundMode);
            if (device)
                device->write(depxEffectMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["DeepBass"]), DeepXOption, [&]() {
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::DeepBassMode);
            if (device)
                device->write(depxEffectMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["ClearTreble"]), DeepXOption, [&]() {
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::ClearTrebleMode);
            if (device)
                device->write(depxEffectMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DeepX"]["options"]["Soothing"]), DeepXOption, [&]() {
            VivoDeviceCommand::DeepxEffectMaker depxEffectMaker(VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode::SoothingMode);
            if (device)
                device->write(depxEffectMaker.data);
        });

        
        auto DoubleClickRightOption = QString::fromStdString(textData["DoubleClickRight"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["PlayPause"]), DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            earDoubleClickMaker.setRightMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode);
            if (device)
                device->write(earDoubleClickMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["Previous"]), DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            earDoubleClickMaker.setRightMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PrevMode);
            if (device)
                device->write(earDoubleClickMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["Next"]), DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            earDoubleClickMaker.setRightMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode);
            if (device)
                device->write(earDoubleClickMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["WakeUpAssistant"]), DoubleClickRightOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            earDoubleClickMaker.setRightMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::WakeupVoiceAssistantMode);
            if (device)
                device->write(earDoubleClickMaker.data);
        });

        auto DoubleClickLeftOption = QString::fromStdString(textData["DoubleClickLeft"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["PlayPause"]), DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            earDoubleClickMaker.setLeftMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PlayOrPauseMode);
            if (device)
                device->write(earDoubleClickMaker.data);

        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["Previous"]), DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            earDoubleClickMaker.setLeftMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::PrevMode);
            if (device)
                device->write(earDoubleClickMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["Next"]), DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            earDoubleClickMaker.setLeftMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode);
            if (device)
                device->write(earDoubleClickMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["WakeUpAssistant"]), DoubleClickLeftOption, [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            earDoubleClickMaker.setLeftMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::WakeupVoiceAssistantMode);
            if (device)
                device->write(earDoubleClickMaker.data);
        });

        /* TOBE FIXED: status record */
        auto PressRightOption = QString::fromStdString(textData["PressRight"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["NoiseSwitch"]), PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseNoiseMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseNoiseMode);
            if (device)
                device->write(earLongPressMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["TransparentSwitch"]), PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseTransparentMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseTransparentMode);
            if (device)
                device->write(earLongPressMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["NoiseTransparentSwitch"]), PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndTransparentMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndTransparentMode);
            if (device)
                device->write(earLongPressMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["NoiseCloseTransparentSwitch"]), PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode);
            if (device)
                device->write(earLongPressMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["None"]), PressRightOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::NoneMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::NoneMode);
            if (device)
                device->write(earLongPressMaker.data);
        });

        /* TOBE FIXED: status record */
        auto PressLeftOption = QString::fromStdString(textData["PressLeft"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["NoiseSwitch"]), PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseNoiseMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode);
            if (device)
                device->write(earLongPressMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["TransparentSwitch"]), PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::OpenCloseTransparentMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode);
            if (device)
                device->write(earLongPressMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["NoiseTransparentSwitch"]), PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndTransparentMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode);
            if (device)
                device->write(earLongPressMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["NoiseCloseTransparentSwitch"]), PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode);
            if (device)
                device->write(earLongPressMaker.data);
        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["None"]), PressLeftOption, [&]() {
            VivoDeviceCommand::EarLongPressMaker earLongPressMaker;
            earLongPressMaker.setMode(VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::NoneMode, VivoDeviceCommand::EarLongPressMaker::EarLongPressMode::SwitchBetweenNoiseAndCloseAndTransparentMode);
            if (device)
                device->write(earLongPressMaker.data);
        });

        /* TOBE FIXED: status record */
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickAcceptCall"]["title"]), [&]() {
            VivoDeviceCommand::EarDoubleClickMaker earDoubleClickMaker;
            earDoubleClickMaker.setLeftMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode);
            earDoubleClickMaker.setRightMode(VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode::NextMode);
            if (device)
                device->write(earDoubleClickMaker.data);
        });

        trayMenu->addOption(QString::fromStdString(textData["PressRefuseCall"]["title"]), [&]() {
            VivoDeviceCommand::AcceptCallMaker acceptCallMaker;
            acceptCallMaker.setMode(VivoDeviceCommand::AcceptCallMaker::DoubleClickMode::DoubleClickAcceprOrRefuseMode, VivoDeviceCommand::AcceptCallMaker::LongPressMode::LongPressRefuseMode);
            if (device)
                device->write(acceptCallMaker.data);
        });

        trayMenu->addOption(QString::fromStdString(textData["WearDetection"]["title"]), [&]() {
            VivoDeviceCommand::WearDetectionMaker wearDetectionMaker(VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OnMode);
            if (device)
                device->write(wearDetectionMaker.data);
        });

        trayMenu->show();
    }
    /* TOBE FIXED: default initialization */
    else {

    }
    /*
    {
    "A": "1", -> acceptCallMaker
    "B": "2_1",
    "C": "92_89",
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
        if(result.size() > 200)
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
