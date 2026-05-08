#include "OrderController.h"

OrderController::OrderController(MainView& view) : view_(view) {}

void OrderController::run() {
    while (true) {
        view_.showOrderManagerMenu(0, 0);
        int input = view_.getMenuInput();
        if (input == 0) break;
        if (input >= 1 && input <= 4)
            view_.showComingSoon();
        else
            view_.showInvalidInput();
    }
}
