#include <gtest/gtest.h>
#include <filesystem>
#include "../model/OrderRepository.h"

namespace fs = std::filesystem;
static const std::string TEST_FILE = "data/test_orders.json";

class OrderRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        fs::create_directories("data");
        if (fs::exists(TEST_FILE)) fs::remove(TEST_FILE);
    }
    void TearDown() override {
        if (fs::exists(TEST_FILE)) fs::remove(TEST_FILE);
    }
};

TEST_F(OrderRepositoryTest, CreateSetsStatusToReserved) {
    OrderRepository repo(TEST_FILE);
    Order o; o.sampleId = 1; o.customerName = "Kim"; o.quantity = 5;
    o.status = OrderStatus::CONFIRMED; // should be overridden
    auto result = repo.create(o);
    EXPECT_EQ(result.status, OrderStatus::RESERVED);
}

TEST_F(OrderRepositoryTest, CreateSetsTimestamps) {
    OrderRepository repo(TEST_FILE);
    Order o; o.sampleId = 1; o.customerName = "Kim"; o.quantity = 3;
    auto result = repo.create(o);
    EXPECT_FALSE(result.createdAt.empty());
    EXPECT_FALSE(result.updatedAt.empty());
    EXPECT_EQ(result.createdAt, result.updatedAt);
}

TEST_F(OrderRepositoryTest, FindAllReturnsCreated) {
    OrderRepository repo(TEST_FILE);
    Order o; o.sampleId = 1; o.customerName = "Lee"; o.quantity = 2;
    repo.create(o);
    EXPECT_EQ(repo.findAll().size(), 1u);
}

TEST_F(OrderRepositoryTest, PersistsAcrossInstances) {
    {
        OrderRepository repo(TEST_FILE);
        Order o; o.sampleId = 2; o.customerName = "Park"; o.quantity = 10;
        repo.create(o);
    }
    OrderRepository repo2(TEST_FILE);
    auto all = repo2.findAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].customerName, "Park");
}

TEST_F(OrderRepositoryTest, RemoveReleasedOrderFails) {
    OrderRepository repo(TEST_FILE);
    Order o; o.sampleId = 1; o.customerName = "X"; o.quantity = 1;
    auto created = repo.create(o);
    repo.updateStatus(created.id, OrderStatus::CONFIRMED);
    repo.updateStatus(created.id, OrderStatus::RELEASED);
    EXPECT_FALSE(repo.remove(created.id));
}

TEST_F(OrderRepositoryTest, UpdateStatusValid) {
    OrderRepository repo(TEST_FILE);
    Order o; o.sampleId = 1; o.customerName = "Y"; o.quantity = 1;
    auto created = repo.create(o);
    EXPECT_TRUE(repo.updateStatus(created.id, OrderStatus::CONFIRMED));
    auto found = repo.findById(created.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->status, OrderStatus::CONFIRMED);
}

TEST_F(OrderRepositoryTest, UpdateStatusInvalid) {
    OrderRepository repo(TEST_FILE);
    Order o; o.sampleId = 1; o.customerName = "Z"; o.quantity = 1;
    auto created = repo.create(o);
    repo.updateStatus(created.id, OrderStatus::CONFIRMED);
    EXPECT_FALSE(repo.updateStatus(created.id, OrderStatus::RESERVED));
}
