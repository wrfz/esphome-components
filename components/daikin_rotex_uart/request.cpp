#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin-uart";

TRequest::TRequest(uint8_t registryID)
: m_registryID(registryID)
, m_last_request_timestamp(0u)
, m_last_response_timestamp(0u)
{
}

uint8_t TRequest::getCRC(std::array<uint8_t, 4> const& data, uint32_t len) {
    uint8_t byte = 0;
    for (int i = 0; i < len; i++) {
        byte += data[i];
    }
    return ~byte;
}

bool TRequest::send(uart::UARTDevice& device) {
    std::array<uint8_t, 4> buffer {0x03, 0x40, m_registryID, 0x00};
    buffer[3] = getCRC(buffer, 3);

    ESP_LOGI(TAG, "TX: %s", Utils::to_hex(buffer).c_str());

    device.write_array(buffer.data(), sizeof(buffer));
    device.flush();

    m_last_request_timestamp = millis();

    return true;
}

void TRequest::setHandled() {
    m_last_response_timestamp = millis();
}

bool TRequest::isInProgress() const {
    uint32_t mil = millis();
    return m_last_request_timestamp > m_last_response_timestamp && ((mil - m_last_request_timestamp) < 3*1000); // Consider 3 sek => package is lost
}

bool TRequest::isRequestRequired() const {
    uint32_t mil = millis();
    uint32_t timeout = 5000u;

    return !isInProgress() && (mil > (m_last_response_timestamp + timeout));
}

} // namespace daikin_rotex_uart
} // namespace esphome