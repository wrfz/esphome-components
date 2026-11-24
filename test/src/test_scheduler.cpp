#include "esphome/components/daikin_rotex_uart/scheduler.h"
#include "mock_esphome.h"
#include <gtest/gtest.h>

using namespace esphome::daikin_rotex_uart;

// The scheduler is a singleton: every test starts at a fixed time and TearDown runs what is left.
// Calls only capture members of the fixture, which still exist in TearDown (a local of the test
// body would be gone by then).
class SchedulerTest : public ::testing::Test {
protected:
    void SetUp() override { esphome::mock::set_millis(1000); }
    void TearDown() override {
        esphome::mock::set_millis(1000000);  // far past every timeout: run the rest
        Scheduler::getInstance().update();
        esphome::mock::set_millis(std::nullopt);
    }

    CallHandle later(uint32_t timeout) {
        return Scheduler::getInstance().call_later([this]() { ++calls; }, timeout);
    }

    int calls = 0;
};

TEST_F(SchedulerTest, call_later_returns_valid_handle) {
    auto handle = later(100);

    EXPECT_TRUE(handle.is_valid());
    EXPECT_EQ(0, calls);
}

TEST_F(SchedulerTest, call_runs_when_due) {
    auto handle = later(100);

    esphome::mock::set_millis(1099);
    Scheduler::getInstance().update();
    EXPECT_EQ(0, calls);

    esphome::mock::set_millis(1100);
    Scheduler::getInstance().update();
    EXPECT_EQ(1, calls);
    EXPECT_FALSE(handle.is_valid());
}

TEST_F(SchedulerTest, cancel) {
    auto handle = later(100);

    EXPECT_TRUE(handle.cancel());
    EXPECT_FALSE(handle.is_valid());
    esphome::mock::set_millis(2000);
    Scheduler::getInstance().update();
    EXPECT_EQ(0, calls);
}

TEST_F(SchedulerTest, accelerate) {
    auto handle = later(100);

    EXPECT_TRUE(handle.accelerate());
    Scheduler::getInstance().update();
    EXPECT_EQ(1, calls);
}

TEST_F(SchedulerTest, due_across_millis_overflow) {
    esphome::mock::set_millis(UINT32_MAX - 49);
    later(100);  // due 50 ms after the overflow

    esphome::mock::set_millis(49);
    Scheduler::getInstance().update();
    EXPECT_EQ(0, calls);
    esphome::mock::set_millis(50);
    Scheduler::getInstance().update();
    EXPECT_EQ(1, calls);
}
