#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../view/SampleView.h"
#include "../view/MainView.h"
#include "../model/IRepository.h"
#include "../model/Sample.h"
#include "../controller/ProductionController.h"

using ::testing::Return;
using ::testing::_;

namespace {

class MockMainView : public MainView {
public:
    MOCK_METHOD(void, showRoleMenu,              (), (override));
    MOCK_METHOD(void, showOrderManagerMenu,      (int, int), (override));
    MOCK_METHOD(void, showProductionManagerMenu, (int, int), (override));
    MOCK_METHOD(void, showComingSoon,            (), (override));
    MOCK_METHOD(void, showInvalidInput,          (), (override));
    MOCK_METHOD(int,  getMenuInput,              (), (override));
};

class MockSampleView : public SampleView {
public:
    MOCK_METHOD(void,        showSampleMenu,          (), (override));
    MOCK_METHOD(std::string, inputName,               (), (override));
    MOCK_METHOD(int,         inputAvgProductionTime,  (), (override));
    MOCK_METHOD(double,      inputYield,              (), (override));
    MOCK_METHOD(void,        showRegistered,   (const Sample&),      (override));
    MOCK_METHOD(void,        showInvalidInput, (const std::string&), (override));
    MOCK_METHOD(void,        showComingSoon,          (), (override));
};

class MockSampleRepository : public IRepository<Sample> {
public:
    MOCK_METHOD(Sample,              create,  (Sample),         (override));
    MOCK_METHOD(std::vector<Sample>, findAll, (),               (override));
    MOCK_METHOD(std::optional<Sample>, findById, (int),         (override));
    MOCK_METHOD(bool,                update,  (const Sample&),  (override));
    MOCK_METHOD(bool,                remove,  (int),            (override));
};

} // namespace

// 실제 호출 순서 (RegisterSampleSuccess):
// run(): showProdMenu → getMenuInput→1(시료관리) → runSampleMenu()
//   runSampleMenu(): showSampleMenu → getMenuInput→1(SM-01) → registerSample()
//     inputName / inputAvgTime / inputYield / create / showRegistered
//   runSampleMenu(): showSampleMenu → getMenuInput→0(종료)
// run(): showProdMenu → getMenuInput→0(종료)

TEST(SM01Test, RegisterSampleSuccess) {
    MockMainView        mainView;
    MockSampleView      sampleView;
    MockSampleRepository repo;

    EXPECT_CALL(mainView, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sampleView, showSampleMenu()).Times(2);
    EXPECT_CALL(mainView, getMenuInput())
        .WillOnce(Return(1))   // run(): 시료 관리 진입
        .WillOnce(Return(1))   // runSampleMenu(): SM-01 선택
        .WillOnce(Return(0))   // runSampleMenu(): 종료
        .WillOnce(Return(0));  // run(): 종료

    EXPECT_CALL(sampleView, inputName()).WillOnce(Return("TestSample"));
    EXPECT_CALL(sampleView, inputAvgProductionTime()).WillOnce(Return(10));
    EXPECT_CALL(sampleView, inputYield()).WillOnce(Return(0.9));

    Sample created; created.id = 1; created.name = "TestSample";
    EXPECT_CALL(repo, create(_)).WillOnce(Return(created));
    EXPECT_CALL(sampleView, showRegistered(_)).Times(1);

    ProductionController ctrl(mainView, sampleView, repo);
    ctrl.run();
}

TEST(SM01Test, RejectsEmptyName) {
    MockMainView        mainView;
    MockSampleView      sampleView;
    MockSampleRepository repo;

    EXPECT_CALL(mainView, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sampleView, showSampleMenu()).Times(2);
    EXPECT_CALL(mainView, getMenuInput())
        .WillOnce(Return(1))   // run(): 시료 관리 진입
        .WillOnce(Return(1))   // runSampleMenu(): SM-01 선택
        .WillOnce(Return(0))   // runSampleMenu(): 종료
        .WillOnce(Return(0));  // run(): 종료

    EXPECT_CALL(sampleView, inputName()).WillOnce(Return(""));
    EXPECT_CALL(sampleView, showInvalidInput(_)).Times(1);
    EXPECT_CALL(repo, create(_)).Times(0);

    ProductionController ctrl(mainView, sampleView, repo);
    ctrl.run();
}

TEST(SM01Test, RejectsNonPositiveAvgTime) {
    MockMainView        mainView;
    MockSampleView      sampleView;
    MockSampleRepository repo;

    EXPECT_CALL(mainView, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sampleView, showSampleMenu()).Times(2);
    EXPECT_CALL(mainView, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(sampleView, inputName()).WillOnce(Return("Valid"));
    EXPECT_CALL(sampleView, inputAvgProductionTime()).WillOnce(Return(0));
    EXPECT_CALL(sampleView, showInvalidInput(_)).Times(1);
    EXPECT_CALL(repo, create(_)).Times(0);

    ProductionController ctrl(mainView, sampleView, repo);
    ctrl.run();
}

TEST(SM01Test, RejectsYieldZero) {
    MockMainView        mainView;
    MockSampleView      sampleView;
    MockSampleRepository repo;

    EXPECT_CALL(mainView, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sampleView, showSampleMenu()).Times(2);
    EXPECT_CALL(mainView, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(sampleView, inputName()).WillOnce(Return("Valid"));
    EXPECT_CALL(sampleView, inputAvgProductionTime()).WillOnce(Return(5));
    EXPECT_CALL(sampleView, inputYield()).WillOnce(Return(0.0));
    EXPECT_CALL(sampleView, showInvalidInput(_)).Times(1);
    EXPECT_CALL(repo, create(_)).Times(0);

    ProductionController ctrl(mainView, sampleView, repo);
    ctrl.run();
}

TEST(SM01Test, RejectsYieldAboveOne) {
    MockMainView        mainView;
    MockSampleView      sampleView;
    MockSampleRepository repo;

    EXPECT_CALL(mainView, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sampleView, showSampleMenu()).Times(2);
    EXPECT_CALL(mainView, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(sampleView, inputName()).WillOnce(Return("Valid"));
    EXPECT_CALL(sampleView, inputAvgProductionTime()).WillOnce(Return(5));
    EXPECT_CALL(sampleView, inputYield()).WillOnce(Return(1.1));
    EXPECT_CALL(sampleView, showInvalidInput(_)).Times(1);
    EXPECT_CALL(repo, create(_)).Times(0);

    ProductionController ctrl(mainView, sampleView, repo);
    ctrl.run();
}
