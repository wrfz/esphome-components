#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "fake_uart.h"
#include "mock_esphome.h"
#include <gtest/gtest.h>

using namespace esphome::daikin_rotex_uart;
using esphome::test::FakeDevice;
using esphome::test::FakeUart;

TEST(RequestTest, crc) {
    // the CRC is the inverted byte sum (frames seen on the bus)
    const uint8_t request[] = {0x03, 0x40, 0x10};
    EXPECT_EQ(0xAC, TRequest::getCRC(request, sizeof(request)));
    const uint8_t request_61[] = {0x03, 0x40, 0x61};
    EXPECT_EQ(0x5B, TRequest::getCRC(request_61, sizeof(request_61)));
    EXPECT_EQ(0xFF, TRequest::getCRC(request, 0));
}

TEST(RequestTest, send_i_protocol_frame) {
    FakeUart uart;
    FakeDevice device(&uart);

    TRequest::sendFrame(device, TProtocol::I, 0x20);

    EXPECT_EQ("03 40 20 9C", Utils::to_hex(uart.tx.data(), uart.tx.size()));
}

TEST(RequestTest, send_s_protocol_frame) {
    FakeUart uart;
    FakeDevice device(&uart);

    TRequest::sendFrame(device, TProtocol::S, 0x53);

    ASSERT_EQ(3u, uart.tx.size());
    EXPECT_EQ(0x02, uart.tx[0]);
    EXPECT_EQ(0x53, uart.tx[1]);
    EXPECT_EQ(TRequest::getCRC(uart.tx.data(), 2), uart.tx[2]);
}

class RequestStateTest : public ::testing::Test {
protected:
    void TearDown() override { esphome::mock::set_millis(std::nullopt); }
};

TEST_F(RequestStateTest, in_progress_until_handled) {
    FakeUart uart;
    FakeDevice device(&uart);
    TRequest request(TProtocol::I, 0x10);

    esphome::mock::set_millis(10000);
    EXPECT_FALSE(request.isInProgress());
    EXPECT_TRUE(request.isRequestRequired());

    request.send(device);
    esphome::mock::set_millis(10500);
    EXPECT_TRUE(request.isInProgress());
    EXPECT_FALSE(request.isRequestRequired());

    request.setHandled();
    EXPECT_FALSE(request.isInProgress());
    EXPECT_EQ(10000u, request.getLastRequestTimestamp());
    EXPECT_EQ(10500u, request.getLastResponeTimestamp());
}

TEST_F(RequestStateTest, lost_after_3_seconds) {
    FakeUart uart;
    FakeDevice device(&uart);
    TRequest request(TProtocol::I, 0x10);

    esphome::mock::set_millis(10000);
    request.send(device);
    esphome::mock::set_millis(12999);
    EXPECT_TRUE(request.isInProgress());
    esphome::mock::set_millis(13000);
    EXPECT_FALSE(request.isInProgress());  // no response: counts as lost
}

TEST_F(RequestStateTest, next_request_5_seconds_after_response) {
    FakeUart uart;
    FakeDevice device(&uart);
    TRequest request(TProtocol::I, 0x10);

    esphome::mock::set_millis(10000);
    request.send(device);
    request.setHandled();

    esphome::mock::set_millis(15000);
    EXPECT_FALSE(request.isRequestRequired());
    esphome::mock::set_millis(15001);
    EXPECT_TRUE(request.isRequestRequired());
}
