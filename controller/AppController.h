#pragma once
#include "OrderController.h"
#include "ProductionController.h"
#include "../view/MainView.h"
#include "../view/SampleView.h"
#include "../model/IRepository.h"
#include "../model/Sample.h"

class AppController {
public:
    AppController(MainView& mainView,
                  SampleView& sampleView,
                  IRepository<Sample>& sampleRepo);
    void run();
private:
    MainView&            mainView_;
    IRepository<Sample>& sampleRepo_;
    OrderController      orderCtrl_;
    ProductionController prodCtrl_;
};
