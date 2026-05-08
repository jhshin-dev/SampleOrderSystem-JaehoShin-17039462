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
    MOCK_METHOD(void, showProductionMenu,     (), (override));
    MOCK_METHOD(void, showProductionStatus,   (const std::vector<ProductionEntry>&), (override));
    MOCK_METHOD(void, showProductionQueue,    (const std::vector<ProductionEntry>&), (override));
    MOCK_METHOD(void, showNoProductionOrders, (), (override));
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

Order makeProducingOrder(int id, int sampleId, int qty, const std::string& updAt) {
    Order o; o.id = id; o.sampleId = sampleId; o.quantity = qty;
    o.status = OrderStatus::PRODUCING; o.customerName = "TestCo";
    o.updatedAt = updAt;
    return o;
}
Sample makeSample(int id, int stock, double yield, int avgTime) {
    Sample s; s.id = id; s.stock = stock; s.yield = yield;
    s.avgProductionTime = avgTime; s.name = "SampleX";
    return s;
}

// 생산 현황·큐 흐름: run()→3(생산라인) → runProductionMenu()
// getMenuInput: run()→3, prodMenu→N, prodMenu→0, run()→0

TEST(PL01Test, ShowsProductionStatus) {
    MockMainView mv; MockSampleView sv; MockOrderView ov;
    MockMonitorView mon; MockProductionView pv;
    MockSampleRepository sr; MockOrderRepository or_;

    Order o = makeProducingOrder(1, 1, 10, "2026-05-08T01:00:00Z");
    Sample s = makeSample(1, 3, 0.9, 5);  // shortage=7, actual=⌈7/0.81⌉=9, time=45

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(pv, showProductionMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(1))   // 생산 현황
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(or_, findAll()).WillOnce(Return(std::vector<Order>{o}));
    EXPECT_CALL(sr, findById(1)).WillOnce(Return(s));
    EXPECT_CALL(pv, showProductionStatus(testing::SizeIs(1))).Times(1);

    ProductionController ctrl(mv, sv, ov, mon, pv, sr, or_);
    ctrl.run();
}

TEST(PL01Test, ShowsNoOrdersWhenEmpty) {
    MockMainView mv; MockSampleView sv; MockOrderView ov;
    MockMonitorView mon; MockProductionView pv;
    MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(pv, showProductionMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(or_, findAll()).WillOnce(Return(std::vector<Order>{}));
    EXPECT_CALL(pv, showNoProductionOrders()).Times(1);

    ProductionController ctrl(mv, sv, ov, mon, pv, sr, or_);
    ctrl.run();
}

TEST(PL02Test, ShowsProductionQueueSortedByUpdatedAt) {
    MockMainView mv; MockSampleView sv; MockOrderView ov;
    MockMonitorView mon; MockProductionView pv;
    MockSampleRepository sr; MockOrderRepository or_;

    // o2가 더 늦게 등록됐으므로 FIFO: o1 먼저
    Order o1 = makeProducingOrder(1, 1, 5, "2026-05-08T01:00:00Z");
    Order o2 = makeProducingOrder(2, 1, 3, "2026-05-08T02:00:00Z");
    Sample s = makeSample(1, 0, 0.9, 10);

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(pv, showProductionMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(2))   // 생산 대기 큐
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(or_, findAll()).WillOnce(Return(std::vector<Order>{o2, o1})); // 순서 뒤바뀐 채로 전달
    EXPECT_CALL(sr, findById(1)).WillRepeatedly(Return(s));
    // 정렬 후 o1이 먼저인 vector가 전달돼야 함
    EXPECT_CALL(pv, showProductionQueue(
        testing::AllOf(
            testing::SizeIs(2),
            testing::Property(&std::vector<ProductionEntry>::front,
                              testing::Field(&ProductionEntry::orderId, 1))
        )
    )).Times(1);

    ProductionController ctrl(mv, sv, ov, mon, pv, sr, or_);
    ctrl.run();
}

TEST(PL02Test, ShowsNoQueueWhenEmpty) {
    MockMainView mv; MockSampleView sv; MockOrderView ov;
    MockMonitorView mon; MockProductionView pv;
    MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(pv, showProductionMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(3))
        .WillOnce(Return(2))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(or_, findAll()).WillOnce(Return(std::vector<Order>{}));
    EXPECT_CALL(pv, showNoProductionOrders()).Times(1);

    ProductionController ctrl(mv, sv, ov, mon, pv, sr, or_);
    ctrl.run();
}

} // namespace
