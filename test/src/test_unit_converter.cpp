#include "esphome/components/daikin_rotex_uart/unit_converter.h"
#include <gtest/gtest.h>

// R410A: gauge pressure of the outdoor unit (bar(g)) -> saturation temperature in tenths of °C.
// The table holds absolute pressure; the converter adds the ambient pressure.

static constexpr double AMBIENT = 1.01325;

TEST(UnitConverterTest, table_points) {
    EXPECT_NEAR(400.0, UnitConverter::liquid_pressure_to_temperature(24.26 - AMBIENT), 1e-6);   // 40 °C
    EXPECT_NEAR(-300.0, UnitConverter::liquid_pressure_to_temperature(2.703 - AMBIENT), 1e-6);  // -30 °C
    EXPECT_NEAR(710.0, UnitConverter::liquid_pressure_to_temperature(48.67 - AMBIENT), 1e-6);   // 71 °C, upper end
}

TEST(UnitConverterTest, interpolates_between_points) {
    // halfway between 40 °C (24.26 bar(a)) and 41 °C (24.85 bar(a))
    EXPECT_NEAR(405.0, UnitConverter::liquid_pressure_to_temperature((24.26 + 24.85) / 2 - AMBIENT), 1e-6);
}

TEST(UnitConverterTest, gauge_pressure_of_the_hpsu) {
    // 18.0 bar(g) = 19.01 bar(a): between 30 °C (18.89 bar(a)) and 31 °C (19.39 bar(a))
    EXPECT_NEAR(302.465, UnitConverter::liquid_pressure_to_temperature(18.0), 1e-3);
    // standstill at about 0 °C: 7.1 bar(g) = 8.11 bar(a)
    EXPECT_NEAR(4.0, UnitConverter::liquid_pressure_to_temperature(7.1), 0.5);
}

TEST(UnitConverterTest, outside_table) {
    EXPECT_EQ(-1, UnitConverter::liquid_pressure_to_temperature(-0.6));  // below 0.482 bar(a)
    EXPECT_EQ(-1, UnitConverter::liquid_pressure_to_temperature(48.0));  // above 48.67 bar(a)
}
