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

// 모니터 흐름: run()→4(모니터링) → runMonitor()
// getMenuInput: run()→4, monitorMenu→N, monitorMenu→0, run()→0

TEST(MO01Test, ShowsOrderStatusExcludingRejected) {
    MockMainView mv; MockOrderView ov; MockMonitorView mv2;
    MockSampleRepository sr; MockOrderRepository or_;

    Order o1; o1.id = 1; o1.status = OrderStatus::RESERVED;
    Order o2; o2.id = 2; o2.status = OrderStatus::REJECTED;
    std::vector<Order> orders = {o1, o2};

    EXPECT_CALL(mv, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mv2, showMonitorMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(4))   // run: 모니터링
        .WillOnce(Return(1))   // monitorMenu: 주문량 확인
        .WillOnce(Return(0))   // monitorMenu: 종료
        .WillOnce(Return(0));  // run: 종료

    EXPECT_CALL(or_, findAll()).WillOnce(Return(orders));
    EXPECT_CALL(mv2, showOrderStatus(orders)).Times(1);

    OrderController ctrl(mv, ov, mv2, sr, or_);
    ctrl.run();
}

TEST(MO01Test, ShowsEmptyOrderStatus) {
    MockMainView mv; MockOrderView ov; MockMonitorView mv2;
    MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mv2, showMonitorMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(4))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(or_, findAll()).WillOnce(Return(std::vector<Order>{}));
    EXPECT_CALL(mv2, showOrderStatus(std::vector<Order>{})).Times(1);

    OrderController ctrl(mv, ov, mv2, sr, or_);
    ctrl.run();
}

TEST(MO02Test, ShowsStockStatus) {
    MockMainView mv; MockOrderView ov; MockMonitorView mv2;
    MockSampleRepository sr; MockOrderRepository or_;

    Sample s; s.id = 1; s.stock = 0;
    std::vector<Sample> samples = {s};
    std::vector<Order>  orders;

    EXPECT_CALL(mv, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mv2, showMonitorMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(4))
        .WillOnce(Return(2))   // monitorMenu: 재고량 확인
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(sr, findAll()).WillOnce(Return(samples));
    EXPECT_CALL(or_, findAll()).WillOnce(Return(orders));
    EXPECT_CALL(mv2, showStockStatus(samples, orders)).Times(1);

    OrderController ctrl(mv, ov, mv2, sr, or_);
    ctrl.run();
}

TEST(MO02Test, ShowsStockStatusWithNoOrders) {
    MockMainView mv; MockOrderView ov; MockMonitorView mv2;
    MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showOrderManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(mv2, showMonitorMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(4))
        .WillOnce(Return(2))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(sr, findAll()).WillOnce(Return(std::vector<Sample>{}));
    EXPECT_CALL(or_, findAll()).WillOnce(Return(std::vector<Order>{}));
    EXPECT_CALL(mv2, showStockStatus(std::vector<Sample>{},
                                      std::vector<Order>{})).Times(1);

    OrderController ctrl(mv, ov, mv2, sr, or_);
    ctrl.run();
}

} // namespace
