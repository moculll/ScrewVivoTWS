#include "ScrewVivoTWS.h"
#include <QtConcurrent>
#include <QPushButton>
#include "Accessibility/AccessMgr.h"
ScrewVivoTWS::ScrewVivoTWS(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    
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
