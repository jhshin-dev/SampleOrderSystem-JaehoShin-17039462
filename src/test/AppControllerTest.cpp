#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../view/MainView.h"
#include "../view/OrderView.h"
#include "../view/MonitorView.h"
#include "../view/ProductionView.h"
#include "../view/SampleView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"
#include "../controller/AppController.h"
#include "../controller/OrderController.h"
#include "../controller/ProductionController.h"

using ::testing::Return;
using ::testing::InSequence;
using ::testing::_;

class MockMainView : public MainView {
public:
    MOCK_METHOD(void, showRoleMenu,              (int, int), (override));
    MOCK_METHOD(void, showOrderManagerMenu,      (int, int), (override));
    MOCK_METHOD(void, showProductionManagerMenu, (int, int), (override));
    MOCK_METHOD(void, showComingSoon,            (), (override));
    MOCK_METHOD(void, showInvalidInput,          (), (override));
    MOCK_METHOD(int,  getMenuInput,              (), (override));
};

class MockOrderView : public OrderView {
public:
    MOCK_METHOD(int,         inputSampleId,      (), (override));
    MOCK_METHOD(std::string, inputCustomerName,  (), (override));
    MOCK_METHOD(int,         inputQuantity,      (), (override));
    MOCK_METHOD(void, showOrderRegistered, (const Order&), (override));
    MOCK_METHOD(void, showOrderList, (const std::vector<Order>&,
                                     const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showNoOrders,      (), (override));
    MOCK_METHOD(void, showInvalidInput,  (const std::string&), (override));
    MOCK_METHOD(void, showComingSoon,    (), (override));
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
    MOCK_METHOD(void,        showSampleList,  (const std::vector<Sample>&), (override));
    MOCK_METHOD(std::string, inputSearchKeyword,      (), (override));
    MOCK_METHOD(void,        showNoResult,            (), (override));
};

class MockSampleRepository : public IRepository<Sample> {
public:
    MOCK_METHOD(Sample,              create,  (Sample),         (override));
    MOCK_METHOD(std::vector<Sample>, findAll, (),               (override));
    MOCK_METHOD(std::optional<Sample>, findById, (int),         (override));
    MOCK_METHOD(bool,                update,  (const Sample&),  (override));
    MOCK_METHOD(bool,                remove,  (int),            (override));
};

class MockOrderRepository : public IOrderRepository {
public:
    MOCK_METHOD(Order,               create,       (Order),           (override));
    MOCK_METHOD(std::vector<Order>,  findAll,      (),                (override));
    MOCK_METHOD(std::optional<Order>,findById,     (int),             (override));
    MOCK_METHOD(bool,                update,       (const Order&),    (override));
    MOCK_METHOD(bool,                remove,       (int),             (override));
    MOCK_METHOD(bool,                updateStatus, (int, OrderStatus),(override));
};

class MockMonitorView : public MonitorView {
public:
    MOCK_METHOD(void, showMonitorMenu,  (), (override));
    MOCK_METHOD(void, showOrderStatus,  (const std::vector<Order>&), (override));
    MOCK_METHOD(void, showStockStatus,  (const std::vector<Sample>&,
                                         const std::vector<Order>&), (override));
};

class MockProductionView : public ProductionView {
public:
    MOCK_METHOD(void, showProductionMenu,     (), (override));
    MOCK_METHOD(void, showProductionStatus,   (const std::vector<ProductionEntry>&), (override));
    MOCK_METHOD(void, showProductionQueue,    (const std::vector<ProductionEntry>&), (override));
    MOCK_METHOD(void, showNoProductionOrders, (), (override));
};

// ── AppController ──────────────────────────────────────────

TEST(AppControllerTest, ExitOnZero) {
    MockMainView mock;
    EXPECT_CALL(mock, showRoleMenu(_, _)).Times(1);
    EXPECT_CALL(mock, getMenuInput()).WillOnce(Return(0));

    MockOrderView ov; MockMonitorView mon; MockSampleView sv;
    MockProductionView pv; MockSampleRepository sr; MockOrderRepository or_;
    EXPECT_CALL(sr, findAll()).WillRepeatedly(Return(std::vector<Sample>{}));
    AppController ctrl(mock, ov, mon, sv, pv, sr, or_);
    ctrl.run();
}

TEST(AppControllerTest, InvalidInputShowsError) {
    MockMainView mock;
    EXPECT_CALL(mock, showRoleMenu(_, _)).Times(2);
    EXPECT_CALL(mock, showInvalidInput()).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(9))
        .WillOnce(Return(0));

    MockOrderView ov; MockMonitorView mon; MockSampleView sv;
    MockProductionView pv; MockSampleRepository sr; MockOrderRepository or_;
    EXPECT_CALL(sr, findAll()).WillRepeatedly(Return(std::vector<Sample>{}));
    AppController ctrl(mock, ov, mon, sv, pv, sr, or_);
    ctrl.run();
}

TEST(AppControllerTest, SelectOrderManager) {
    MockMainView mock;
    EXPECT_CALL(mock, showRoleMenu(_, _)).Times(2);
    EXPECT_CALL(mock, showOrderManagerMenu(0, 0)).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(1))   // 역할 선택: 주문 담당자
        .WillOnce(Return(0))   // 주문 담당자 메뉴: 종료
        .WillOnce(Return(0));  // 역할 선택: 종료

    MockOrderView ov; MockMonitorView mon; MockSampleView sv;
    MockProductionView pv; MockSampleRepository sr; MockOrderRepository or_;
    EXPECT_CALL(sr, findAll()).WillRepeatedly(Return(std::vector<Sample>{}));
    AppController ctrl(mock, ov, mon, sv, pv, sr, or_);
    ctrl.run();
}

TEST(AppControllerTest, SelectProductionManager) {
    MockMainView mock;
    EXPECT_CALL(mock, showRoleMenu(_, _)).Times(2);
    EXPECT_CALL(mock, showProductionManagerMenu(0, 0)).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(2))   // 역할 선택: 생산 담당자
        .WillOnce(Return(0))   // 생산 담당자 메뉴: 종료
        .WillOnce(Return(0));  // 역할 선택: 종료

    MockOrderView ov; MockMonitorView mon; MockSampleView sv;
    MockProductionView pv; MockSampleRepository sr; MockOrderRepository or_;
    EXPECT_CALL(sr, findAll()).WillRepeatedly(Return(std::vector<Sample>{}));
    AppController ctrl(mock, ov, mon, sv, pv, sr, or_);
    ctrl.run();
}

// ── OrderController ────────────────────────────────────────

TEST(OrderControllerTest, ExitOnZero) {
    MockMainView mock; MockOrderView ov; MockMonitorView mon;
    MockSampleRepository sr; MockOrderRepository or_;
    EXPECT_CALL(mock, showOrderManagerMenu(0, 0)).Times(1);
    EXPECT_CALL(mock, getMenuInput()).WillOnce(Return(0));

    OrderController ctrl(mock, ov, mon, sr, or_);
    ctrl.run();
}

TEST(OrderControllerTest, InvalidInputShowsError) {
    MockMainView mock; MockOrderView ov; MockMonitorView mon;
    MockSampleRepository sr; MockOrderRepository or_;
    EXPECT_CALL(mock, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mock, showInvalidInput()).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(9))
        .WillOnce(Return(0));

    OrderController ctrl(mock, ov, mon, sr, or_);
    ctrl.run();
}

// ── ProductionController ───────────────────────────────────

TEST(ProductionControllerTest, ExitOnZero) {
    MockMainView mock; MockSampleView sv; MockOrderView ov;
    MockMonitorView mon; MockProductionView pv; MockSampleRepository sr; MockOrderRepository or_;
    EXPECT_CALL(mock, showProductionManagerMenu(0, 0)).Times(1);
    EXPECT_CALL(mock, getMenuInput()).WillOnce(Return(0));

    ProductionController ctrl(mock, sv, ov, mon, pv, sr, or_);
    ctrl.run();
}

TEST(ProductionControllerTest, InvalidInputShowsError) {
    MockMainView mock; MockSampleView sv; MockOrderView ov;
    MockMonitorView mon; MockProductionView pv; MockSampleRepository sr; MockOrderRepository or_;
    EXPECT_CALL(mock, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mock, showInvalidInput()).Times(1);
    EXPECT_CALL(mock, getMenuInput())
        .WillOnce(Return(9))
        .WillOnce(Return(0));

    ProductionController ctrl(mock, sv, ov, mon, pv, sr, or_);
    ctrl.run();
}
