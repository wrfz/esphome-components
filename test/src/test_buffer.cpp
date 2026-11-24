#include "esphome/components/daikin_rotex_uart/buffer.h"
#include "fake_uart.h"
#include <gtest/gtest.h>

using namespace esphome::daikin_rotex_uart;
using esphome::test::FakeDevice;
using esphome::test::FakeUart;

TEST(BufferTest, read_collects_parts) {
    // since ESPHome 2025.10 a response arrives in parts (rx_full_threshold): the buffer appends them
    FakeUart uart;
    FakeDevice device(&uart);
    TBuffer buffer;

    uart.receive({0x40, 0x20, 0x13, 0x78, 0x00, 0x00, 0x00});
    EXPECT_EQ("40 20 13 78 00 00 00", buffer.read(device));  // returns only the part just read
    uart.receive({0x59, 0x01});
    EXPECT_EQ("59 01", buffer.read(device));

    EXPECT_EQ(9u, buffer.size());
    EXPECT_EQ(0x40, buffer[0]);
    EXPECT_EQ(0x01, buffer[8]);
}

TEST(BufferTest, read_nothing_available) {
    FakeUart uart;
    FakeDevice device(&uart);
    TBuffer buffer;

    EXPECT_EQ("", buffer.read(device));
    EXPECT_EQ(0u, buffer.size());
}

TEST(BufferTest, read_stops_at_capacity) {
    FakeUart uart;
    FakeDevice device(&uart);
    TBuffer buffer;

    uart.receive(std::vector<uint8_t>(70, 0xAA));
    buffer.read(device);
    EXPECT_EQ(buffer.data().size(), buffer.size());   // 64: full
    EXPECT_EQ(6u, uart.available());                  // the rest stays in the UART

    buffer.shift(10);
    buffer.read(device);
    EXPECT_EQ(60u, buffer.size());
    EXPECT_EQ(0u, uart.available());
}

TEST(BufferTest, shift_keeps_the_rest) {
    FakeUart uart;
    FakeDevice device(&uart);
    TBuffer buffer;

    uart.receive({0x01, 0x02, 0x03, 0x04, 0x05});
    buffer.read(device);
    buffer.shift(2);

    EXPECT_EQ(3u, buffer.size());
    EXPECT_EQ(0x03, buffer[0]);
    EXPECT_EQ(0x05, buffer[2]);
}

TEST(BufferTest, clear) {
    FakeUart uart;
    FakeDevice device(&uart);
    TBuffer buffer;

    uart.receive({0x01, 0x02});
    buffer.read(device);
    buffer.clear();
    EXPECT_EQ(0u, buffer.size());
}
