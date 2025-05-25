#pragma once

#include <QtWidgets/QWidget>
#include "ui_ScrewVivoTWS.h"
#include "VivoController/VivoController.h"
class ScrewVivoTWS : public QWidget
{
    Q_OBJECT

public:
    ScrewVivoTWS(QWidget *parent = nullptr);
    ~ScrewVivoTWS();
    std::unique_ptr<VivoDevice> device;
private:
    Ui::ScrewVivoTWSClass ui;
};
