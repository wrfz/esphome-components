#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin-uart";

TRequest::TRequest(TProtocol protocol, uint8_t registryID)
: m_protocol(protocol)
, m_registryID(registryID)
, m_last_request_timestamp(0u)
, m_last_response_timestamp(0u)
{
}

uint8_t TRequest::getCRC(uint8_t const* data, uint32_t len) {
    uint8_t byte = 0;
    for (uint32_t i = 0; i < len; i++) {
        byte += data[i];
    }
    return ~byte;
}

bool TRequest::sendFrame(uart::UARTDevice& device, TProtocol protocol, uint8_t registryID) {
    if (protocol == TProtocol::S) {
        std::array<uint8_t, 3> buffer {0x02, registryID, 0x00};
        buffer[2] = getCRC(buffer.data(), sizeof(buffer) - 1);

        ESP_LOGI(TAG, "TX[S]: %s", Utils::to_hex(buffer).c_str());

        device.write_array(buffer.data(), sizeof(buffer));
        device.flush();

        return true;
    }

    std::array<uint8_t, 4> buffer {0x03, 0x40, registryID, 0x00};
    buffer[3] = getCRC(buffer.data(), sizeof(buffer) - 1);

    ESP_LOGI(TAG, "TX: %s", Utils::to_hex(buffer).c_str());

    device.write_array(buffer.data(), sizeof(buffer));
    device.flush();

    return true;
}

bool TRequest::send(uart::UARTDevice& device) {
    sendFrame(device, m_protocol, m_registryID);

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