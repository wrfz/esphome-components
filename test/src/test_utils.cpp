#include "esphome/components/daikin_rotex_uart/utils.h"
#include <gtest/gtest.h>

#include <array>

using namespace esphome::daikin_rotex_uart;

TEST(UtilsTest, to_hex_array) {
    std::array<uint8_t, 4> data{0x03, 0x40, 0x10, 0xAC};
    EXPECT_EQ("03 40 10 AC", Utils::to_hex(data));
    EXPECT_EQ("03 40", Utils::to_hex(data, 2));
    EXPECT_EQ("03 40 10 AC", Utils::to_hex(data, 10));  // maxlen beyond the array: whole array
}

TEST(UtilsTest, to_hex_pointer) {
    const uint8_t data[] = {0x00, 0x0A, 0xFF};
    EXPECT_EQ("00 0A FF", Utils::to_hex(data, 3));
    EXPECT_EQ("00", Utils::to_hex(data, 1));
    EXPECT_EQ("", Utils::to_hex(data, 0));
}

TEST(UtilsTest, to_hex_value) {
    EXPECT_EQ("0x01", Utils::to_hex(1u));
    EXPECT_EQ("0x0A", Utils::to_hex(10u));
    EXPECT_EQ("0xFF", Utils::to_hex(255u));
    EXPECT_EQ("0x10A", Utils::to_hex(0x10Au));
}

TEST(UtilsTest, str_to_map) {
    auto map = Utils::str_to_map("0x00:Aus|0x01:An|0x1B:str 2");
    EXPECT_EQ(3u, map.size());
    EXPECT_EQ("Aus", map[0x00]);
    EXPECT_EQ("An", map[0x01]);
    EXPECT_EQ("str 2", map[0x1B]);
}

TEST(UtilsTest, str_to_map_ignores_entries_without_colon) {
    auto map = Utils::str_to_map("0x01:a|broken|0x02:b");
    EXPECT_EQ(2u, map.size());
    EXPECT_EQ("a", map[0x01]);
    EXPECT_EQ("b", map[0x02]);
}

TEST(UtilsTest, hash_str) {
    // constexpr: usable in switch/case
    static_assert(Utils::hash_str("tv") != Utils::hash_str("tr"));
    EXPECT_EQ(Utils::hash_str("temperature_spread"), Utils::hash_str(std::string("temperature_spread")));
    EXPECT_NE(Utils::hash_str("thermal_power"), Utils::hash_str("temperature_spread"));
}

TEST(UtilsTest, format) {
    EXPECT_EQ("a: 1, b: x", Utils::format("a: %d, b: %s", 1, "x"));
    EXPECT_EQ("2.50", Utils::format("%.2f", 2.5));
}

TEST(UtilsTest, equals) {
    EXPECT_TRUE(Utils::equals(1.0f, 1.05f, 0.1f));
    EXPECT_FALSE(Utils::equals(1.0f, 1.2f, 0.1f));
}
