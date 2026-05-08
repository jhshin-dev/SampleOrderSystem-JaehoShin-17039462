#pragma once
#include "../view/MainView.h"
#include "../view/SampleView.h"
#include "../model/IRepository.h"
#include "../model/Sample.h"

class ProductionController {
public:
    ProductionController(MainView& mainView,
                         SampleView& sampleView,
                         IRepository<Sample>& sampleRepo);
    void run();
private:
    MainView&            mainView_;
    SampleView&          sampleView_;
    IRepository<Sample>& sampleRepo_;

    void runSampleMenu();
    void registerSample();
};
