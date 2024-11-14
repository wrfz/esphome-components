#pragma once

#include <stdint.h>
#include <array>

namespace esphome {

namespace uart {
    class UARTDevice;
}

namespace daikin_rotex_uart {

class TRequest {
public:
    TRequest(uint8_t registryID);

    bool send(uart::UARTDevice& device);
    bool isInProgress() const;
    bool isRequestRequired() const;
    void setHandled();
    uint8_t getRegistryId() const;
private:
    static uint8_t getCRC(std::array<uint8_t, 4> const& data, uint32_t len);

    uint8_t m_registryID;
    uint32_t m_last_request_timestamp;
    uint32_t m_last_response_timestamp;
};

inline uint8_t TRequest::getRegistryId() const {
    return m_registryID;
}

} // namespace daikin_rotex_uart
} // namespace esphome