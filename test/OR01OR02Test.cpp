#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../view/MainView.h"
#include "../view/OrderView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"
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
    MOCK_METHOD(int,         inputSampleId,       (), (override));
    MOCK_METHOD(std::string, inputCustomerName,   (), (override));
    MOCK_METHOD(int,         inputQuantity,       (), (override));
    MOCK_METHOD(void,        showOrderRegistered, (const Order&), (override));
    MOCK_METHOD(void,        showOrderList,       (const std::vector<Order>&,
                                                   const std::vector<Sample>&), (override));
    MOCK_METHOD(void,        showNoOrders,        (), (override));
    MOCK_METHOD(void,        showInvalidInput,    (const std::string&), (override));
    MOCK_METHOD(void,        showComingSoon,      (), (override));
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
    MOCK_METHOD(Order,                create,       (Order),        (override));
    MOCK_METHOD(std::vector<Order>,   findAll,      (),             (override));
    MOCK_METHOD(std::optional<Order>, findById,     (int),          (override));
    MOCK_METHOD(bool,                 update,       (const Order&), (override));
    MOCK_METHOD(bool,                 remove,       (int),          (override));
    MOCK_METHOD(bool,                 updateStatus, (int, OrderStatus), (override));
};

// OR-01·02 흐름: run() → menu → 입력 → 처리 → menu → 0 종료
// getMenuInput 호출 순서: showOrderManagerMenu → N선택 → ... → showOrderManagerMenu → 0

TEST(OR01Test, ReceiveOrderSuccess) {
    MockMainView mv; MockOrderView ov; MockSampleRepository sr; MockOrderRepository or_;

    Sample s; s.id = 1; s.name = "Silicon-A";
    Order created; created.id = 1; created.sampleId = 1;
    created.customerName = "Kim"; created.quantity = 5;

    EXPECT_CALL(mv, showOrderManagerMenu(_, _)).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(0));

    EXPECT_CALL(ov, inputSampleId()).WillOnce(Return(1));
    EXPECT_CALL(sr, findById(1)).WillOnce(Return(s));
    EXPECT_CALL(ov, inputCustomerName()).WillOnce(Return("Kim"));
    EXPECT_CALL(ov, inputQuantity()).WillOnce(Return(5));
    EXPECT_CALL(or_, create(_)).WillOnce(Return(created));
    EXPECT_CALL(ov, showOrderRegistered(_)).Times(1);

    OrderController ctrl(mv, ov, sr, or_);
    ctrl.run();
}

TEST(OR01Test, RejectsInvalidSampleId) {
    MockMainView mv; MockOrderView ov; MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showOrderManagerMenu(_, _)).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(0));
    EXPECT_CALL(ov, inputSampleId()).WillOnce(Return(99));
    EXPECT_CALL(sr, findById(99)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(ov, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, create(_)).Times(0);

    OrderController ctrl(mv, ov, sr, or_);
    ctrl.run();
}

TEST(OR01Test, RejectsEmptyCustomerName) {
    MockMainView mv; MockOrderView ov; MockSampleRepository sr; MockOrderRepository or_;

    Sample s; s.id = 1;
    EXPECT_CALL(mv, showOrderManagerMenu(_, _)).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(0));
    EXPECT_CALL(ov, inputSampleId()).WillOnce(Return(1));
    EXPECT_CALL(sr, findById(1)).WillOnce(Return(s));
    EXPECT_CALL(ov, inputCustomerName()).WillOnce(Return(""));
    EXPECT_CALL(ov, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, create(_)).Times(0);

    OrderController ctrl(mv, ov, sr, or_);
    ctrl.run();
}

TEST(OR01Test, RejectsZeroQuantity) {
    MockMainView mv; MockOrderView ov; MockSampleRepository sr; MockOrderRepository or_;

    Sample s; s.id = 1;
    EXPECT_CALL(mv, showOrderManagerMenu(_, _)).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(0));
    EXPECT_CALL(ov, inputSampleId()).WillOnce(Return(1));
    EXPECT_CALL(sr, findById(1)).WillOnce(Return(s));
    EXPECT_CALL(ov, inputCustomerName()).WillOnce(Return("Kim"));
    EXPECT_CALL(ov, inputQuantity()).WillOnce(Return(0));
    EXPECT_CALL(ov, showInvalidInput(_)).Times(1);
    EXPECT_CALL(or_, create(_)).Times(0);

    OrderController ctrl(mv, ov, sr, or_);
    ctrl.run();
}

TEST(OR02Test, ShowsReservedOrders) {
    MockMainView mv; MockOrderView ov; MockSampleRepository sr; MockOrderRepository or_;

    Order o1; o1.id = 1; o1.sampleId = 1; o1.status = OrderStatus::RESERVED;
    std::vector<Order> orders = {o1};
    std::vector<Sample> samples;

    EXPECT_CALL(mv, showOrderManagerMenu(_, _)).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(2))
        .WillOnce(Return(0));
    EXPECT_CALL(or_, findAll()).WillOnce(Return(orders));
    EXPECT_CALL(sr, findAll()).WillOnce(Return(samples));
    EXPECT_CALL(ov, showOrderList(_, _)).Times(1);

    OrderController ctrl(mv, ov, sr, or_);
    ctrl.run();
}

TEST(OR02Test, ShowsEmptyWhenNoOrders) {
    MockMainView mv; MockOrderView ov; MockSampleRepository sr; MockOrderRepository or_;

    EXPECT_CALL(mv, showOrderManagerMenu(_, _)).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(2))
        .WillOnce(Return(0));
    EXPECT_CALL(or_, findAll()).WillOnce(Return(std::vector<Order>{}));
    EXPECT_CALL(ov, showNoOrders()).Times(1);

    OrderController ctrl(mv, ov, sr, or_);
    ctrl.run();
}

} // namespace
