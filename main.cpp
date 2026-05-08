#include <gtest/gtest.h>
#include "controller/AppController.h"
#include "view/MainView.h"
#include "view/OrderView.h"
#include "view/MonitorView.h"
#include "view/SampleView.h"
#include "model/SampleRepository.h"
#include "model/OrderRepository.h"
#include <string>

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]).rfind("--gtest", 0) == 0) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }
    SampleRepository sampleRepo;
    OrderRepository  orderRepo;
    MainView    mainView;
    OrderView   orderView;
    MonitorView monitorView;
    SampleView  sampleView;
    AppController app(mainView, orderView, monitorView, sampleView, sampleRepo, orderRepo);
    app.run();
    return 0;
}
