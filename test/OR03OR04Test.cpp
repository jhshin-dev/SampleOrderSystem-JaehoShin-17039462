#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../view/MainView.h"
#include "../view/OrderView.h"
#include "../view/SampleView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"
#include "../model/Order.h"
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

// 헬퍼: 주문 생성
Order makeOrder(int id, int sampleId, int qty, OrderStatus status = OrderStatus::RESERVED) {
    Order o; o.id = id; o.sampleId = sampleId;
    o.quantity = qty; o.status = status;
    o.customerName = "TestCo";
    return o;
}
Sample makeSample(int id, int stock) {
    Sample s; s.id = id; s.stock = stock; s.name = "Silicon-A";
    s.avgProductionTime = 10; s.yield = 0.9;
    return s;
}

// 승인·거절 흐름: run()→2(주문승인거절) → runOrderApprovalMenu()
// getMenuInput: run()→2, approvalMenu→선택, approvalMenu→0, run()→0

TEST(OR03Test, ApprovesOrderConfirmedWhenStockSufficient) {
    MockMainView mv; MockOrderView ov; MockSampleView sv;
    MockSampleRepository sr; MockOrderRepository or_;

    Order o   = makeOrder(1, 1, 5);
    Sample s  = makeSample(1, 10); // stock(10) >= qty(5)

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showApprovalMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(2))   // run: 주문 승인·거절
        .WillOnce(Return(2))   // approvalMenu: 주문 승인
        .WillOnce(Return(0))   // approvalMenu: 종료
        .WillOnce(Return(0));  // run: 종료

    EXPECT_CALL(ov, inputOrderId()).WillOnce(Return(1));
    EXPECT_CALL(or_, findById(1)).WillOnce(Return(o));
    EXPECT_CALL(sr, findById(1)).WillOnce(Return(s));
    EXPECT_CALL(or_, updateStatus(1, OrderStatus::CONFIRMED)).WillOnce(Return(true));
    EXPECT_CALL(ov, showConfirmed(_)).Times(1);

    ProductionController ctrl(mv, sv, ov, sr, or_);
    ctrl.run();
}

TEST(OR03Test, ApprovesOrderProducingWhenStockInsufficient) {
    MockMainView mv; MockOrderView ov; MockSampleView sv;
    MockSampleRepository sr; MockOrderRepository or_;

    Order o  = makeOrder(1, 1, 10);
    Sample s = makeSample(1, 3); // stock(3) < qty(10)

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showApprovalMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(2))
        .WillOnce(Return(2))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(ov, inputOrderId()).WillOnce(Return(1));
    EXPECT_CALL(or_, findById(1)).WillOnce(Return(o));
    EXPECT_CALL(sr, findById(1)).WillOnce(Return(s));
    EXPECT_CALL(or_, updateStatus(1, OrderStatus::PRODUCING)).WillOnce(Return(true));
    EXPECT_CALL(ov, showSentToProduction(_)).Times(1);

    ProductionController ctrl(mv, sv, ov, sr, or_);
    ctrl.run();
}

TEST(OR03Test, RejectsApprovalForNonExistentOrder) {
    MockMainView mv; MockOrderView ov; MockSampleView sv;
    MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showApprovalMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(2))
        .WillOnce(Return(2))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(ov, inputOrderId()).WillOnce(Return(99));
    EXPECT_CALL(or_, findById(99)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(ov, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, updateStatus(_, _)).Times(0);

    ProductionController ctrl(mv, sv, ov, sr, or_);
    ctrl.run();
}

TEST(OR03Test, RejectsApprovalForNonReservedOrder) {
    MockMainView mv; MockOrderView ov; MockSampleView sv;
    MockSampleRepository sr; MockOrderRepository or_;

    Order confirmed = makeOrder(1, 1, 5, OrderStatus::CONFIRMED);

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showApprovalMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(2))
        .WillOnce(Return(2))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(ov, inputOrderId()).WillOnce(Return(1));
    EXPECT_CALL(or_, findById(1)).WillOnce(Return(confirmed));
    EXPECT_CALL(ov, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, updateStatus(_, _)).Times(0);

    ProductionController ctrl(mv, sv, ov, sr, or_);
    ctrl.run();
}

TEST(OR04Test, RejectsOrder) {
    MockMainView mv; MockOrderView ov; MockSampleView sv;
    MockSampleRepository sr; MockOrderRepository or_;

    Order o = makeOrder(1, 1, 5);

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showApprovalMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(2))
        .WillOnce(Return(3))   // 주문 거절
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(ov, inputOrderId()).WillOnce(Return(1));
    EXPECT_CALL(or_, findById(1)).WillOnce(Return(o));
    EXPECT_CALL(or_, updateStatus(1, OrderStatus::REJECTED)).WillOnce(Return(true));
    EXPECT_CALL(ov, showRejected(_)).Times(1);

    ProductionController ctrl(mv, sv, ov, sr, or_);
    ctrl.run();
}

TEST(OR04Test, RejectsRejectionForNonReservedOrder) {
    MockMainView mv; MockOrderView ov; MockSampleView sv;
    MockSampleRepository sr; MockOrderRepository or_;

    Order confirmed = makeOrder(2, 1, 5, OrderStatus::CONFIRMED);

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(ov, showApprovalMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(2))
        .WillOnce(Return(3))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(ov, inputOrderId()).WillOnce(Return(2));
    EXPECT_CALL(or_, findById(2)).WillOnce(Return(confirmed));
    EXPECT_CALL(ov, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, updateStatus(_, _)).Times(0);

    ProductionController ctrl(mv, sv, ov, sr, or_);
    ctrl.run();
}

} // namespace
