#pragma once

#include "esphome/components/daikin_rotex_uart/protocol.h"
#include <stdint.h>
#include <array>

namespace esphome {

namespace uart {
    class UARTDevice;
}

namespace daikin_rotex_uart {

class TRequest {
public:
    TRequest(TProtocol protocol, uint8_t registryID);

    bool send(uart::UARTDevice& device);
    bool isInProgress() const;
    bool isRequestRequired() const;
    void setHandled();
    uint8_t getRegistryId() const;
    uint32_t getLastRequestTimestamp() const;
    uint32_t getLastResponeTimestamp() const;
private:
    static uint8_t getCRC(uint8_t const* data, uint32_t len);
    static bool sendFrame(uart::UARTDevice& device, TProtocol protocol, uint8_t registryID);

    TProtocol m_protocol;
    uint8_t m_registryID;
    uint32_t m_last_request_timestamp;
    uint32_t m_last_response_timestamp;
};

inline uint8_t TRequest::getRegistryId() const {
    return m_registryID;
}

inline uint32_t TRequest::getLastRequestTimestamp() const {
    return m_last_request_timestamp;
}

inline uint32_t TRequest::getLastResponeTimestamp() const {
    return m_last_response_timestamp;
}

} // namespace daikin_rotex_uart
} // namespace esphome