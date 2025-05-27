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
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["Previous"]), DoubleClickRightOption, [&]() {
           
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["Next"]), DoubleClickRightOption, [&]() {
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickRight"]["options"]["WakeUpAssistant"]), DoubleClickRightOption, [&]() {
            
        });

        auto DoubleClickLeftOption = QString::fromStdString(textData["DoubleClickLeft"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["PlayPause"]), DoubleClickLeftOption, [&]() {
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["Previous"]), DoubleClickLeftOption, [&]() {
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["Next"]), DoubleClickLeftOption, [&]() {
            
        });
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickLeft"]["options"]["WakeUpAssistant"]), DoubleClickLeftOption, [&]() {
          
        });

        auto PressRightOption = QString::fromStdString(textData["PressRight"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["NoiseSwitch"]), PressRightOption, [&]() {
           
        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["TransparentSwitch"]), PressRightOption, [&]() {
            
        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["NoiseTransparentSwitch"]), PressRightOption, [&]() {
           
        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["NoiseCloseTransparentSwitch"]), PressRightOption, [&]() {
            
        });
        trayMenu->addOption(QString::fromStdString(textData["PressRight"]["options"]["None"]), PressRightOption, [&]() {
            
        });

        auto PressLeftOption = QString::fromStdString(textData["PressLeft"]["title"]);
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["title"]), nullptr);
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["NoiseSwitch"]), PressLeftOption, [&]() {
            
        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["TransparentSwitch"]), PressLeftOption, [&]() {
            
        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["NoiseTransparentSwitch"]), PressLeftOption, [&]() {
            
        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["NoiseCloseTransparentSwitch"]), PressLeftOption, [&]() {
          
        });
        trayMenu->addOption(QString::fromStdString(textData["PressLeft"]["options"]["None"]), PressLeftOption, [&]() {
            
        });

        // Double-tap Answer/End Call
        trayMenu->addOption(QString::fromStdString(textData["DoubleClickAcceptCall"]["title"]), [&]() {
            
        });

        trayMenu->addOption(QString::fromStdString(textData["PressRefuseCall"]["title"]), [&]() {

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

    /* qtimer runs in qt main thread, so we can use bluetooth device in it */
    readAsyncTimer = std::make_unique<QTimer>();
    QObject::connect(readAsyncTimer.get(), &QTimer::timeout, [&]() {
        if (!device->isConnected())
            return;
        auto result = device->read();
        spdlog::info("read: {} bytes.", result.size());
        
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
