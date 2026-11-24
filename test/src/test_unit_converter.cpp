#include "esphome/components/daikin_rotex_uart/unit_converter.h"
#include <gtest/gtest.h>

// R410A: liquid pressure (bar) -> saturation temperature in tenths of °C

TEST(UnitConverterTest, table_points) {
    EXPECT_NEAR(400.0, UnitConverter::liquid_pressure_to_temperature(24.26), 1e-6);   // 40 °C
    EXPECT_NEAR(-300.0, UnitConverter::liquid_pressure_to_temperature(2.703), 1e-6);  // -30 °C
    EXPECT_NEAR(710.0, UnitConverter::liquid_pressure_to_temperature(48.67), 1e-6);   // 71 °C, upper end
}

TEST(UnitConverterTest, interpolates_between_points) {
    // halfway between 40 °C (24.26 bar) and 41 °C (24.85 bar)
    EXPECT_NEAR(405.0, UnitConverter::liquid_pressure_to_temperature((24.26 + 24.85) / 2), 1e-6);
}

TEST(UnitConverterTest, outside_table) {
    EXPECT_EQ(-1, UnitConverter::liquid_pressure_to_temperature(0.4));
    EXPECT_EQ(-1, UnitConverter::liquid_pressure_to_temperature(50.0));
}
