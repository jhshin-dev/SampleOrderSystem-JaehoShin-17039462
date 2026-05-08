#include "AppController.h"
#include <windows.h>

AppController::AppController(MainView& mainView,
                             SampleView& sampleView,
                             IRepository<Sample>& sampleRepo)
    : mainView_(mainView),
      orderCtrl_(mainView),
      prodCtrl_(mainView, sampleView, sampleRepo) {}

void AppController::run() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    while (true) {
        mainView_.showRoleMenu();
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)
            orderCtrl_.run();
        else if (input == 2)
            prodCtrl_.run();
        else
            mainView_.showInvalidInput();
    }
}
