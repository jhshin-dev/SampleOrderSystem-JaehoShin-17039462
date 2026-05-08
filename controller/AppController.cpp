#include "AppController.h"
#include <windows.h>

AppController::AppController(MainView& view)
    : view_(view), orderCtrl_(view), prodCtrl_(view) {}

void AppController::run() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    while (true) {
        view_.showRoleMenu();
        int input = view_.getMenuInput();
        if (input == 0) break;
        if (input == 1)
            orderCtrl_.run();
        else if (input == 2)
            prodCtrl_.run();
        else
            view_.showInvalidInput();
    }
}
