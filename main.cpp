#include <gtest/gtest.h>
#include "controller/AppController.h"
#include "view/MainView.h"
#include <string>

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]).rfind("--gtest", 0) == 0) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }
    MainView view;
    AppController app(view);
    app.run();
    return 0;
}
