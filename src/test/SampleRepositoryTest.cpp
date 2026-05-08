#include <gtest/gtest.h>
#include <filesystem>
#include "../model/SampleRepository.h"

namespace fs = std::filesystem;

static const std::string TEST_FILE = "data/test_samples.json";

class SampleRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        fs::create_directories("data");
        if (fs::exists(TEST_FILE)) fs::remove(TEST_FILE);
    }
    void TearDown() override {
        if (fs::exists(TEST_FILE)) fs::remove(TEST_FILE);
    }
};

TEST_F(SampleRepositoryTest, FindAllEmptyOnStart) {
    SampleRepository repo(TEST_FILE);
    EXPECT_TRUE(repo.findAll().empty());
}

TEST_F(SampleRepositoryTest, CreateAssignsAutoIncrementId) {
    SampleRepository repo(TEST_FILE);
    Sample s1; s1.name = "A";
    Sample s2; s2.name = "B";
    auto r1 = repo.create(s1);
    auto r2 = repo.create(s2);
    EXPECT_EQ(r1.id, 1);
    EXPECT_EQ(r2.id, 2);
}

TEST_F(SampleRepositoryTest, StockAlwaysInitializedToZero) {
    SampleRepository repo(TEST_FILE);
    Sample s; s.name = "X"; s.stock = 999;
    auto result = repo.create(s);
    EXPECT_EQ(result.stock, 0);
}

TEST_F(SampleRepositoryTest, FindAllReturnsCreatedItems) {
    SampleRepository repo(TEST_FILE);
    Sample s; s.name = "Alpha"; s.avgProductionTime = 10; s.yield = 0.9;
    repo.create(s);
    auto all = repo.findAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].name, "Alpha");
}

TEST_F(SampleRepositoryTest, FindByIdReturnsItem) {
    SampleRepository repo(TEST_FILE);
    Sample s; s.name = "Beta";
    auto created = repo.create(s);
    auto found = repo.findById(created.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Beta");
}

TEST_F(SampleRepositoryTest, FindByIdReturnsNulloptForMissing) {
    SampleRepository repo(TEST_FILE);
    EXPECT_FALSE(repo.findById(999).has_value());
}

TEST_F(SampleRepositoryTest, PersistsAcrossInstances) {
    {
        SampleRepository repo(TEST_FILE);
        Sample s; s.name = "Persist"; s.avgProductionTime = 5; s.yield = 0.8;
        repo.create(s);
    }
    SampleRepository repo2(TEST_FILE);
    auto all = repo2.findAll();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].name, "Persist");
    EXPECT_EQ(all[0].avgProductionTime, 5);
    EXPECT_DOUBLE_EQ(all[0].yield, 0.8);
}
