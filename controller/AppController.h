#pragma once
#include "OrderController.h"
#include "ProductionController.h"
#include "../view/MainView.h"
#include "../view/OrderView.h"
#include "../view/SampleView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"

class AppController {
public:
    AppController(MainView& mainView,
                  OrderView& orderView,
                  SampleView& sampleView,
                  IRepository<Sample>& sampleRepo,
                  IOrderRepository& orderRepo);
    void run();
private:
    MainView&            mainView_;
    IRepository<Sample>& sampleRepo_;
    OrderController      orderCtrl_;
    ProductionController prodCtrl_;
};
