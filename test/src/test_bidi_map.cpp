#include "esphome/components/daikin_rotex_uart/BidiMap.h"
#include <gtest/gtest.h>

using namespace esphome::daikin_rotex_uart;

TEST(BidiMapTest, findByKey) {
    BidiMap bidi_map(std::map<uint16_t, std::string>{{1, "a"}, {3, "c"}});

    EXPECT_EQ("a", bidi_map.findByKey(1)->second);
    EXPECT_EQ("c", bidi_map.findByKey(3)->second);
    EXPECT_TRUE(bidi_map.findByKey(2) == bidi_map.end());
}

TEST(BidiMapTest, findByValue) {
    BidiMap bidi_map{{1, "a"}, {3, "c"}};

    EXPECT_EQ(1, bidi_map.findByValue("a")->first);
    EXPECT_EQ(3, bidi_map.findByValue("c")->first);
    EXPECT_TRUE(bidi_map.findByValue("b") == bidi_map.end());
    EXPECT_TRUE(bidi_map.findByValue("") == bidi_map.end());
}

TEST(BidiMapTest, findNextByKey_nearest) {
    // nearest key; on a tie the smaller one
    BidiMap bidi_map{{1, "a"}, {3, "c"}, {5, "e"}, {7, "g"}};

    EXPECT_EQ("a", bidi_map.findNextByKey(0)->second);
    EXPECT_EQ("a", bidi_map.findNextByKey(1)->second);
    EXPECT_EQ("a", bidi_map.findNextByKey(2)->second);
    EXPECT_EQ("c", bidi_map.findNextByKey(3)->second);
    EXPECT_EQ("e", bidi_map.findNextByKey(6)->second);
    EXPECT_EQ("g", bidi_map.findNextByKey(7)->second);
    EXPECT_EQ("g", bidi_map.findNextByKey(200)->second);
}

TEST(BidiMapTest, findNextByKey_empty) {
    BidiMap bidi_map;
    EXPECT_TRUE(bidi_map.findNextByKey(1) == bidi_map.end());
}

TEST(BidiMapTest, assign) {
    BidiMap bidi_map{{2, "x"}};
    bidi_map = std::map<uint16_t, std::string>{{1, "a"}, {3, "c"}};

    EXPECT_EQ("a", bidi_map.findByKey(1)->second);
    EXPECT_EQ(3, bidi_map.findByValue("c")->first);
    // the previous content is gone in both directions
    EXPECT_TRUE(bidi_map.findByKey(2) == bidi_map.end());
    EXPECT_TRUE(bidi_map.findByValue("x") == bidi_map.end());
}

TEST(BidiMapTest, string) {
    BidiMap bidi_map{{1, "a"}, {3, "c"}};
    EXPECT_EQ("{1, a} {3, c} ", bidi_map.string());
}
