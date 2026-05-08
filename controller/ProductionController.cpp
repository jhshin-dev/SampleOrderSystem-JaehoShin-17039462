#include "ProductionController.h"

ProductionController::ProductionController(MainView& mainView,
                                           SampleView& sampleView,
                                           IRepository<Sample>& sampleRepo)
    : mainView_(mainView), sampleView_(sampleView), sampleRepo_(sampleRepo) {}

void ProductionController::run() {
    while (true) {
        mainView_.showProductionManagerMenu(0, 0);
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)
            runSampleMenu();
        else if (input >= 2 && input <= 4)
            mainView_.showComingSoon();
        else
            mainView_.showInvalidInput();
    }
}

void ProductionController::runSampleMenu() {
    while (true) {
        sampleView_.showSampleMenu();
        int input = mainView_.getMenuInput();
        if (input == 0) break;
        if (input == 1)
            registerSample();
        else if (input >= 2 && input <= 3)
            sampleView_.showComingSoon();
        else
            mainView_.showInvalidInput();
    }
}

void ProductionController::registerSample() {
    std::string name = sampleView_.inputName();
    if (name.empty()) {
        sampleView_.showInvalidInput("이름을 입력해주세요.");
        return;
    }

    int avgTime = sampleView_.inputAvgProductionTime();
    if (avgTime <= 0) {
        sampleView_.showInvalidInput("평균 생산시간은 1분 이상이어야 합니다.");
        return;
    }

    double yield = sampleView_.inputYield();
    if (yield <= 0.0 || yield > 1.0) {
        sampleView_.showInvalidInput("수율은 0.0 초과 1.0 이하로 입력해주세요.");
        return;
    }

    Sample s;
    s.name              = name;
    s.avgProductionTime = avgTime;
    s.yield             = yield;

    Sample created = sampleRepo_.create(s);
    sampleView_.showRegistered(created);
}
