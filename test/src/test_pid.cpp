#include "esphome/components/daikin_rotex_uart/pid.h"
#include "mock_esphome.h"
#include <gtest/gtest.h>

#include <cmath>
#include <limits>

using namespace esphome::daikin_rotex_uart;

class PIDTest : public ::testing::Test {
protected:
    void TearDown() override { esphome::mock::set_millis(std::nullopt); }
};

TEST_F(PIDTest, moves_towards_setpoint) {
    PID pid(0.2f, 0.05f, 0.05f, 0.2f, 0.2f, 0.1f);
    std::string logstr;
    float current = 0.f;

    for (int i = 0; i < 3; ++i) {
        const float before = current;
        current += pid.compute(5.f, current, 10.f, logstr);
        EXPECT_GT(current, before);
        EXPECT_LT(current, 5.f);
    }
}

TEST_F(PIDTest, integral_is_limited) {
    // max_integral 0.2 with i 0.05: the I part stays at most 0.01, however large the error is
    PID pid(0.f, 0.05f, 0.f, 0.2f, 1.f, 1.f);
    std::string logstr;

    float output = 0.f;
    for (int i = 0; i < 10; ++i) {
        output = pid.compute(1000.f, 0.f, 10.f, logstr);
    }
    EXPECT_NEAR(0.01f, output, 1e-6f);
}

TEST_F(PIDTest, invalid_input_gives_no_output) {
    PID pid(0.2f, 0.05f, 0.05f, 0.2f, 0.2f, 0.1f);
    std::string logstr;
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();

    EXPECT_EQ(0.f, pid.compute(nan, 0.f, 10.f, logstr));
    EXPECT_EQ(0.f, pid.compute(1.f, nan, 10.f, logstr));
    EXPECT_EQ(0.f, pid.compute(1.f, 0.f, inf, logstr));
    EXPECT_EQ(0.f, pid.compute(1.f, 0.f, 0.f, logstr));  // dt 0
}

TEST_F(PIDTest, remembers_last_update) {
    PID pid(0.2f, 0.05f, 0.05f, 0.2f, 0.2f, 0.1f);
    std::string logstr;
    EXPECT_EQ(0u, pid.get_last_update());

    esphome::mock::set_millis(4711);
    pid.compute(1.f, 0.f, 10.f, logstr);
    EXPECT_EQ(4711u, pid.get_last_update());

    // no update for invalid input (NaN current value)
    esphome::mock::set_millis(9000);
    pid.compute(1.f, std::numeric_limits<float>::quiet_NaN(), 10.f, logstr);
    EXPECT_EQ(4711u, pid.get_last_update());
}
