#include <gtest/gtest.h>
#include "controller/AppController.h"
#include "view/MainView.h"
#include "view/SampleView.h"
#include "model/SampleRepository.h"
#include <string>

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]).rfind("--gtest", 0) == 0) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }
    SampleRepository sampleRepo;
    MainView   mainView;
    SampleView sampleView;
    AppController app(mainView, sampleView, sampleRepo);
    app.run();
    return 0;
}
