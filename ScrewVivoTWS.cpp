#include "ScrewVivoTWS.h"
#include <QtConcurrent>
#include <QPushButton>
ScrewVivoTWS::ScrewVivoTWS(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    
    
    
    device = VivoController::init();
    device->connect();

    connect(ui.sendBtn, &QPushButton::clicked, [&]() {
    
        VivoDeviceCommand::WearDetectionMaker doubleClick;

        
        doubleClick.setMode(VivoDeviceCommand::WearDetectionMaker::WearDetectionMode::OffMode);
        printf("data: ");
        for (int i = 0; i < doubleClick.data.size(); i++) {
            printf("%02x ", doubleClick.data[i]);
        }
        printf("\r\n");
        device->write(doubleClick.data);
        
        
    });
    
    
    
    
}

ScrewVivoTWS::~ScrewVivoTWS()
{}
