#pragma once
#include "../view/MainView.h"

class ProductionController {
public:
    explicit ProductionController(MainView& view);
    void run();
private:
    MainView& view_;
};
