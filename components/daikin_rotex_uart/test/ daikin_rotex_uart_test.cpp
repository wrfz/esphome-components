#include <gtest/gtest.h>
#include "components/daikin_rotex_uart/daikin_rotex_uart.h"

using namespace esphome::daikin_rotex_uart;

TEST(DaikinRotexUART, TestSendData) {
    DaikinRotexUART device;
    std::vector<uint8_t> data_to_send = {0x01, 0x02, 0x03};
  
    device.send_data(data_to_send);

    ASSERT_EQ(data_to_send.size(), 3);
}

TEST(DaikinRotexUART, TestReceiveData) {
    DaikinRotexUART device;
  
    std::vector<uint8_t> simulated_received = {0x0A, 0x0B, 0x0C};
    for (auto byte : simulated_received) {
        device.write(byte);
    }

    auto received_data = device.receive_data();
    ASSERT_EQ(received_data, simulated_received);
}