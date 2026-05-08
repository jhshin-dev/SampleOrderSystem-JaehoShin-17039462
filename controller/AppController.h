#pragma once
#include "OrderController.h"
#include "ProductionController.h"
#include "../view/MainView.h"

class AppController {
public:
    explicit AppController(MainView& view);
    void run();
private:
    MainView&            view_;
    OrderController      orderCtrl_;
    ProductionController prodCtrl_;
};
