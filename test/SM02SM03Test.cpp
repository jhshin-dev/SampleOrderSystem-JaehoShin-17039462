#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../view/SampleView.h"
#include "../view/MainView.h"
#include "../view/OrderView.h"
#include "../view/MonitorView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"
#include "../model/Order.h"
#include "../controller/ProductionController.h"
#include "../controller/AppController.h"

using ::testing::Return;
using ::testing::_;

namespace {

class MockOrderView : public OrderView {
public:
    MOCK_METHOD(int,         inputSampleId,      (), (override));
    MOCK_METHOD(std::string, inputCustomerName,  (), (override));
    MOCK_METHOD(int,         inputQuantity,      (), (override));
    MOCK_METHOD(void, showOrderRegistered, (const Order&), (override));
    MOCK_METHOD(void, showOrderList, (const std::vector<Order>&,
                                     const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showNoOrders,     (), (override));
    MOCK_METHOD(void, showInvalidInput, (const std::string&), (override));
    MOCK_METHOD(void, showComingSoon,   (), (override));
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
    MOCK_METHOD(void,        showSampleMenu,          (), (override));
    MOCK_METHOD(std::string, inputName,               (), (override));
    MOCK_METHOD(int,         inputAvgProductionTime,  (), (override));
    MOCK_METHOD(double,      inputYield,              (), (override));
    MOCK_METHOD(void,        showRegistered,   (const Sample&),      (override));
    MOCK_METHOD(void,        showInvalidInput, (const std::string&), (override));
    MOCK_METHOD(void,        showComingSoon,          (), (override));
    MOCK_METHOD(void,        showSampleList,   (const std::vector<Sample>&), (override));
    MOCK_METHOD(std::string, inputSearchKeyword,      (), (override));
    MOCK_METHOD(void,        showNoResult,            (), (override));
};

class MockSampleRepository : public IRepository<Sample> {
public:
    MOCK_METHOD(Sample,                create,  (Sample),         (override));
    MOCK_METHOD(std::vector<Sample>,   findAll, (),               (override));
    MOCK_METHOD(std::optional<Sample>, findById,(int),            (override));
    MOCK_METHOD(bool,                  update,  (const Sample&),  (override));
    MOCK_METHOD(bool,                  remove,  (int),            (override));
};

Sample makeSample(int id, const std::string& name) {
    Sample s; s.id = id; s.name = name;
    s.avgProductionTime = 10; s.yield = 0.9; s.stock = 0;
    return s;
}

} // namespace

// ── SM-02 시료 조회 ────────────────────────────────────────

// 시료 조회 흐름: run()→1(시료관리) → runSampleMenu()→2(조회) → listSamples()
//   → runSampleMenu()→0 → run()→0

TEST(SM02Test, ShowsAllSamples) {
    MockMainView mv; MockSampleView sv; MockSampleRepository repo;

    std::vector<Sample> samples = { makeSample(1, "Silicon-A"), makeSample(2, "Germanium-B") };

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sv, showSampleMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(1))   // run(): 시료 관리
        .WillOnce(Return(2))   // runSampleMenu(): 조회
        .WillOnce(Return(0))   // runSampleMenu(): 종료
        .WillOnce(Return(0));  // run(): 종료

    EXPECT_CALL(repo, findAll()).WillOnce(Return(samples));
    EXPECT_CALL(sv, showSampleList(samples)).Times(1);

    MockOrderView ov2; MockMonitorView mon2; MockOrderRepository or2_;
    ProductionController ctrl(mv, sv, ov2, mon2, repo, or2_);
    ctrl.run();
}

TEST(SM02Test, ShowsEmptyListWhenNoSamples) {
    MockMainView mv; MockSampleView sv; MockSampleRepository repo;

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sv, showSampleMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(2))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(repo, findAll()).WillOnce(Return(std::vector<Sample>{}));
    EXPECT_CALL(sv, showSampleList(std::vector<Sample>{})).Times(1);

    MockOrderView ov2; MockMonitorView mon2; MockOrderRepository or2_;
    ProductionController ctrl(mv, sv, ov2, mon2, repo, or2_);
    ctrl.run();
}

// ── SM-03 시료 검색 ────────────────────────────────────────

TEST(SM03Test, FindsByKeyword) {
    MockMainView mv; MockSampleView sv; MockSampleRepository repo;

    std::vector<Sample> all = { makeSample(1, "Silicon-A"), makeSample(2, "Germanium-B") };
    std::vector<Sample> expected = { makeSample(1, "Silicon-A") };

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sv, showSampleMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(3))   // runSampleMenu(): 검색
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(repo, findAll()).WillOnce(Return(all));
    EXPECT_CALL(sv, inputSearchKeyword()).WillOnce(Return("Silicon"));
    EXPECT_CALL(sv, showSampleList(testing::Truly([](const std::vector<Sample>& v) {
        return v.size() == 1 && v[0].name == "Silicon-A";
    }))).Times(1);

    MockOrderView ov2; MockMonitorView mon2; MockOrderRepository or2_;
    ProductionController ctrl(mv, sv, ov2, mon2, repo, or2_);
    ctrl.run();
}

TEST(SM03Test, ShowsNoResultWhenNotFound) {
    MockMainView mv; MockSampleView sv; MockSampleRepository repo;

    std::vector<Sample> all = { makeSample(1, "Silicon-A") };

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sv, showSampleMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(3))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(repo, findAll()).WillOnce(Return(all));
    EXPECT_CALL(sv, inputSearchKeyword()).WillOnce(Return("Gallium"));
    EXPECT_CALL(sv, showNoResult()).Times(1);

    MockOrderView ov2; MockMonitorView mon2; MockOrderRepository or2_;
    ProductionController ctrl(mv, sv, ov2, mon2, repo, or2_);
    ctrl.run();
}

TEST(SM03Test, EmptyKeywordMatchesAll) {
    MockMainView mv; MockSampleView sv; MockSampleRepository repo;

    std::vector<Sample> all = { makeSample(1, "Silicon-A"), makeSample(2, "Germanium-B") };

    EXPECT_CALL(mv, showProductionManagerMenu(0, 0)).Times(2);
    EXPECT_CALL(sv, showSampleMenu()).Times(2);
    EXPECT_CALL(mv, getMenuInput())
        .WillOnce(Return(1))
        .WillOnce(Return(3))
        .WillOnce(Return(0))
        .WillOnce(Return(0));

    EXPECT_CALL(repo, findAll()).WillOnce(Return(all));
    EXPECT_CALL(sv, inputSearchKeyword()).WillOnce(Return(""));
    EXPECT_CALL(sv, showSampleList(testing::Truly([](const std::vector<Sample>& v) {
        return v.size() == 2;
    }))).Times(1);

    MockOrderView ov2; MockMonitorView mon2; MockOrderRepository or2_;
    ProductionController ctrl(mv, sv, ov2, mon2, repo, or2_);
    ctrl.run();
}

// ── 요약 정보 동적 계산 ────────────────────────────────────

TEST(AppControllerSummaryTest, ShowsCountAndStockFromRepo) {
    MockMainView mv; MockSampleView sv; MockSampleRepository repo;

    std::vector<Sample> samples;
    Sample s1; s1.id = 1; s1.stock = 10; s1.name = "A";
    Sample s2; s2.id = 2; s2.stock = 5;  s2.name = "B";
    samples.push_back(s1); samples.push_back(s2);

    EXPECT_CALL(repo, findAll()).WillRepeatedly(Return(samples));
    EXPECT_CALL(mv, showRoleMenu(2, 15)).Times(1);
    EXPECT_CALL(mv, getMenuInput()).WillOnce(Return(0));

    MockOrderView ov; MockMonitorView mon; MockOrderRepository or_;
    AppController ctrl(mv, ov, mon, sv, repo, or_);
    ctrl.run();
}

TEST(AppControllerSummaryTest, ShowsZeroWhenRepoEmpty) {
    MockMainView mv; MockSampleView sv; MockSampleRepository repo;

    EXPECT_CALL(repo, findAll()).WillRepeatedly(Return(std::vector<Sample>{}));
    EXPECT_CALL(mv, showRoleMenu(0, 0)).Times(1);
    EXPECT_CALL(mv, getMenuInput()).WillOnce(Return(0));

    MockOrderView ov; MockMonitorView mon; MockOrderRepository or_;
    AppController ctrl(mv, ov, mon, sv, repo, or_);
    ctrl.run();
}
