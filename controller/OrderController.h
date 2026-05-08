#pragma once
#include "../view/MainView.h"

class OrderController {
public:
    explicit OrderController(MainView& view);
    void run();
private:
    MainView& view_;
};
