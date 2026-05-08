#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../view/MainView.h"
#include "../view/SampleView.h"
#include "../view/OrderView.h"
#include "../view/MonitorView.h"
#include "../view/ProductionView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"
#include "../model/Order.h"
#include "../model/ProductionEntry.h"
#include "../controller/ProductionController.h"

using ::testing::Return;
using ::testing::_;

namespace {

class MockMainView : public MainView {
public:
    MOCK_METHOD(void, showRoleMenu,              (int, int), (override));
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
    MOCK_METHOD(void,        showSampleList,  (const std::vector<Sample>&), (override));
    MOCK_METHOD(std::string, inputSearchKeyword,      (), (override));
    MOCK_METHOD(void,        showNoResult,            (), (override));
};

class MockOrderView : public OrderView {
public:
    MOCK_METHOD(int,         inputSampleId,         (), (override));
    MOCK_METHOD(std::string, inputCustomerName,     (), (override));
    MOCK_METHOD(int,         inputQuantity,         (), (override));
    MOCK_METHOD(void, showOrderRegistered,  (const Order&), (override));
    MOCK_METHOD(void, showOrderList, (const std::vector<Order>&,
                                     const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showNoOrders,         (), (override));
    MOCK_METHOD(void, showInvalidInput,     (const std::string&), (override));
    MOCK_METHOD(void, showComingSoon,       (), (override));
    MOCK_METHOD(void, showApprovalMenu,     (), (override));
    MOCK_METHOD(int,  inputOrderId,         (), (override));
    MOCK_METHOD(void, showConfirmed,        (const Order&), (override));
    MOCK_METHOD(void, showSentToProduction, (const Order&), (override));
    MOCK_METHOD(void, showRejected,         (const Order&), (override));
    MOCK_METHOD(void, showReleaseMenu,      (), (override));
    MOCK_METHOD(void, showReleased,         (const Order&), (override));
    MOCK_METHOD(void, showNoConfirmedOrders,(), (override));
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
    MOCK_METHOD(void, showProductionMenu,      (), (override));
    MOCK_METHOD(void, showProductionStatus,    (const std::vector<ProductionEntry>&), (override));
    MOCK_METHOD(void, showProductionQueue,     (const std::vector<ProductionEntry>&), (override));
    MOCK_METHOD(void, showNoProductionOrders,  (), (override));
    MOCK_METHOD(int,  inputOrderId,            (), (override));
    MOCK_METHOD(void, showProductionCompleted, (const Order&, int), (override));
    MOCK_METHOD(void, showInvalidInput,        (const std::string&), (override));
};

class MockSampleRepository : public IRepository<Sample> {
public:
    MOCK_METHOD(Sample,                create,  (Sample),        (override));
    MOCK_METHOD(std::vector<Sample>,   findAll, (),              (override));
    MOCK_METHOD(std::optional<Sample>, findById,(int),           (override));
    MOCK_METHOD(bool,                  update,  (const Sample&), (override));
    MOCK_METHOD(bool,                  remove,  (int),           (override));
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

// PL-03 흐름: run()→3(생산라인) → runProductionMenu()→3(완료처리)
// getMenuInput: run()→3, prodMenu→3, prodMenu→0, run()→0

TEST(PL03Test, CompletesProduction) {
    MockMainView mv; MockSampleView sv; MockOrderView ov;
    MockMonitorView mon; MockProductionView pv;
    MockSampleRepository sr; MockOrderRepository or_;

    Order producing; producing.id = 1; producing.sampleId = 1;
    producing.quantity = 10; producing.status = OrderStatus::PRODUCING;
    producing.customerName = "TestCo";

    Sample s; s.id = 1; s.stock = 3; s.yield = 0.9; s.avgProductionTime = 5;
    s.name = "Silicon-A";
    // shortage=10-3=7, actualQty=⌈7/0.81⌉=9

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(pv, showProductionMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))   // run: 생산 라인
        .WillOnce(Return(3))   // prodMenu: 생산 완료 처리
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(pv, inputOrderId()).WillOnce(Return(1));
    EXPECT_CALL(or_, findById(1)).WillOnce(Return(producing));
    EXPECT_CALL(sr, findById(1)).WillOnce(Return(s));
    EXPECT_CALL(or_, updateStatus(1, OrderStatus::CONFIRMED)).WillOnce(Return(true));
    // stock: 3 + 9 = 12
    EXPECT_CALL(sr, update(testing::Field(&Sample::stock, 12))).WillOnce(Return(true));
    EXPECT_CALL(pv, showProductionCompleted(_, 9)).Times(1);

    ProductionController ctrl(mv, sv, ov, mon, pv, sr, or_);
    ctrl.run();
}

TEST(PL03Test, RejectsNonProducingOrder) {
    MockMainView mv; MockSampleView sv; MockOrderView ov;
    MockMonitorView mon; MockProductionView pv;
    MockSampleRepository sr; MockOrderRepository or_;

    Order confirmed; confirmed.id = 1; confirmed.status = OrderStatus::CONFIRMED;

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(pv, showProductionMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(3))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(pv, inputOrderId()).WillOnce(Return(1));
    EXPECT_CALL(or_, findById(1)).WillOnce(Return(confirmed));
    EXPECT_CALL(pv, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, updateStatus(_, _)).Times(0);

    ProductionController ctrl(mv, sv, ov, mon, pv, sr, or_);
    ctrl.run();
}

TEST(PL03Test, RejectsNonExistentOrder) {
    MockMainView mv; MockSampleView sv; MockOrderView ov;
    MockMonitorView mon; MockProductionView pv;
    MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(pv, showProductionMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(3))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(pv, inputOrderId()).WillOnce(Return(99));
    EXPECT_CALL(or_, findById(99)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(pv, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, updateStatus(_, _)).Times(0);

    ProductionController ctrl(mv, sv, ov, mon, pv, sr, or_);
    ctrl.run();
}

} // namespace
