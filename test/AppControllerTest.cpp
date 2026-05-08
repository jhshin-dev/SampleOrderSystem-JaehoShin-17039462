#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../view/MainView.h"
#include "../view/SampleView.h"
#include "../model/IRepository.h"
#include "../model/Sample.h"
#include "../controller/AppController.h"
#include "../controller/OrderController.h"
#include "../controller/ProductionController.h"

using ::testing::Return;
using ::testing::InSequence;

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
    MOCK_METHOD(void,        showSampleMenu,         (), (override));
    MOCK_METHOD(std::string, inputName,              (), (override));
    MOCK_METHOD(int,         inputAvgProductionTime, (), (override));
    MOCK_METHOD(double,      inputYield,             (), (override));
    MOCK_METHOD(void,        showRegistered,  (const Sample&),      (override));
    MOCK_METHOD(void,        showInvalidInput,(const std::string&), (override));
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

// ── AppController ──────────────────────────────────────────

TEST(AppControllerTest, ExitOnZero) {
    MockMainView mock;
    EXPECT_CALL(mock, showRoleMenu()).Times(1);
    EXPECT_CALL(mock, getMenuInput()).WillOnce(Return(0));

    MockSampleView sv; MockSampleRepository sr;
    AppController ctrl(mock, sv, sr);
    ctrl.run();
}

TEST(AppControllerTest, InvalidInputShowsError) {
    MockMainView mock;
    EXPECT_CALL(mock, showRoleMenu()).Times(2);
    EXPECT_CALL(mock, showInvalidInput()).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(9))
        .WillOnce(Return(0));

    MockSampleView sv; MockSampleRepository sr;
    AppController ctrl(mock, sv, sr);
    ctrl.run();
}

TEST(AppControllerTest, SelectOrderManager) {
    MockMainView mock;
    EXPECT_CALL(mock, showRoleMenu()).Times(2);
    EXPECT_CALL(mock, showOrderManagerMenu(0, 0)).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(1))   // 역할 선택: 주문 담당자
        .WillOnce(Return(0))   // 주문 담당자 메뉴: 종료
        .WillOnce(Return(0));  // 역할 선택: 종료

    MockSampleView sv; MockSampleRepository sr;
    AppController ctrl(mock, sv, sr);
    ctrl.run();
}

TEST(AppControllerTest, SelectProductionManager) {
    MockMainView mock;
    EXPECT_CALL(mock, showRoleMenu()).Times(2);
    EXPECT_CALL(mock, showProductionManagerMenu(0, 0)).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(2))   // 역할 선택: 생산 담당자
        .WillOnce(Return(0))   // 생산 담당자 메뉴: 종료
        .WillOnce(Return(0));  // 역할 선택: 종료

    MockSampleView sv; MockSampleRepository sr;
    AppController ctrl(mock, sv, sr);
    ctrl.run();
}

// ── OrderController ────────────────────────────────────────

TEST(OrderControllerTest, ExitOnZero) {
    MockMainView mock;
    EXPECT_CALL(mock, showOrderManagerMenu(0, 0)).Times(1);
    EXPECT_CALL(mock, getMenuInput()).WillOnce(Return(0));

    OrderController ctrl(mock);
    ctrl.run();
}

TEST(OrderControllerTest, ComingSoonOnValidMenu) {
    MockMainView mock;
    EXPECT_CALL(mock, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mock, showComingSoon()).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(0));

    OrderController ctrl(mock);
    ctrl.run();
}

TEST(OrderControllerTest, InvalidInputShowsError) {
    MockMainView mock;
    EXPECT_CALL(mock, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mock, showInvalidInput()).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(9))
        .WillOnce(Return(0));

    OrderController ctrl(mock);
    ctrl.run();
}

// ── ProductionController ───────────────────────────────────

TEST(ProductionControllerTest, ExitOnZero) {
    MockMainView mock; MockSampleView sv; MockSampleRepository sr;
    EXPECT_CALL(mock, showProductionManagerMenu(0, 0)).Times(1);
    EXPECT_CALL(mock, getMenuInput()).WillOnce(Return(0));

    ProductionController ctrl(mock, sv, sr);
    ctrl.run();
}

TEST(ProductionControllerTest, ComingSoonOnValidMenu) {
    MockMainView mock; MockSampleView sv; MockSampleRepository sr;
    EXPECT_CALL(mock, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mock, showComingSoon()).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(2))   // 2~4 → 준비 중 (1은 시료 관리로 진입)
        .WillOnce(Return(0));

    ProductionController ctrl(mock, sv, sr);
    ctrl.run();
}

TEST(ProductionControllerTest, InvalidInputShowsError) {
    MockMainView mock; MockSampleView sv; MockSampleRepository sr;
    EXPECT_CALL(mock, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mock, showInvalidInput()).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(9))
        .WillOnce(Return(0));

    ProductionController ctrl(mock, sv, sr);
    ctrl.run();
}
