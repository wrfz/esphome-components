#include "esphome/components/daikin_rotex_uart/request.h"

uint8_t getCRC(std::array<uint8_t, 4> const& src, uint32_t len)
{
    uint8_t b = 0;
    for (int i = 0; i < len; i++)
    {
        b += src[i];
    }
    return ~b;
}

void TRequest::send(uart::UARTDevice& device) {
    uint8_t regID = 0x60;

    std::array<uint8_t, 4> prep {0x03, 0x40, regID, 0x00};
    prep[3] = getCRC(prep, 3);
    const size_t queryLength = 4;

    ESP_LOGI(TAG, "write: %s", Utils::to_hex(prep).c_str());

    device.flush();
    device.write_array(prep.data(), queryLength);
}