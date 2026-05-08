#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../view/MainView.h"
#include "../view/OrderView.h"
#include "../view/MonitorView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"
#include "../model/Order.h"
#include "../controller/OrderController.h"

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

Order makeOrder(int id, int sampleId, int qty,
                OrderStatus st = OrderStatus::CONFIRMED) {
    Order o; o.id = id; o.sampleId = sampleId;
    o.quantity = qty; o.status = st;
    o.customerName = "TestCo";
    return o;
}

// 출고 처리 흐름: run()→3(출고처리) → runReleaseMenu()
// getMenuInput: run()→3, releaseMenu→N, releaseMenu→0, run()→0

TEST(RL01Test, ShowsConfirmedOrders) {
    MockMainView mv; MockOrderView ov; MockMonitorView mon;
    MockSampleRepository sr; MockOrderRepository or_;

    Order c1 = makeOrder(1, 1, 5, OrderStatus::CONFIRMED);
    Order r1 = makeOrder(2, 1, 3, OrderStatus::RESERVED);  // 제외돼야 함
    std::vector<Order> all = {c1, r1};

    EXPECT_CALL(mv, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showReleaseMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))   // run: 출고 처리
        .WillOnce(Return(1))   // release: CONFIRMED 목록
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(or_, findAll()).WillOnce(Return(all));
    EXPECT_CALL(sr, findAll()).WillOnce(Return(std::vector<Sample>{}));
    EXPECT_CALL(ov, showOrderList(testing::SizeIs(1), _)).Times(1);

    OrderController ctrl(mv, ov, mon, sr, or_);
    ctrl.run();
}

TEST(RL01Test, ShowsNoOrdersWhenNoneConfirmed) {
    MockMainView mv; MockOrderView ov; MockMonitorView mon;
    MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showReleaseMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(or_, findAll()).WillOnce(Return(std::vector<Order>{}));
    EXPECT_CALL(ov, showNoConfirmedOrders()).Times(1);

    OrderController ctrl(mv, ov, mon, sr, or_);
    ctrl.run();
}

TEST(RL02Test, ExecutesRelease) {
    MockMainView mv; MockOrderView ov; MockMonitorView mon;
    MockSampleRepository sr; MockOrderRepository or_;

    Order confirmed = makeOrder(1, 1, 5, OrderStatus::CONFIRMED);
    Sample s; s.id = 1; s.stock = 10;

    EXPECT_CALL(mv, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showReleaseMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(2))   // release: 출고 실행
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(ov, inputOrderId()).WillOnce(Return(1));
    EXPECT_CALL(or_, findById(1)).WillOnce(Return(confirmed));
    EXPECT_CALL(or_, updateStatus(1, OrderStatus::RELEASED)).WillOnce(Return(true));
    EXPECT_CALL(sr, findById(1)).WillOnce(Return(s));
    // stock(10) - qty(5) = 5
    EXPECT_CALL(sr, update(testing::Field(&Sample::stock, 5))).WillOnce(Return(true));
    EXPECT_CALL(ov, showReleased(_)).Times(1);

    OrderController ctrl(mv, ov, mon, sr, or_);
    ctrl.run();
}

TEST(RL02Test, RejectsReleaseForNonConfirmedOrder) {
    MockMainView mv; MockOrderView ov; MockMonitorView mon;
    MockSampleRepository sr; MockOrderRepository or_;

    Order reserved = makeOrder(1, 1, 5, OrderStatus::RESERVED);

    EXPECT_CALL(mv, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showReleaseMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(2))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(ov, inputOrderId()).WillOnce(Return(1));
    EXPECT_CALL(or_, findById(1)).WillOnce(Return(reserved));
    EXPECT_CALL(ov, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, updateStatus(_, _)).Times(0);

    OrderController ctrl(mv, ov, mon, sr, or_);
    ctrl.run();
}

TEST(RL02Test, RejectsReleaseForNonExistentOrder) {
    MockMainView mv; MockOrderView ov; MockMonitorView mon;
    MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showReleaseMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(2))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(ov, inputOrderId()).WillOnce(Return(99));
    EXPECT_CALL(or_, findById(99)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(ov, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, updateStatus(_, _)).Times(0);

    OrderController ctrl(mv, ov, mon, sr, or_);
    ctrl.run();
}

} // namespace
