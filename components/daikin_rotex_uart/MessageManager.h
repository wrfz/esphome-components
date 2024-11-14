#pragma once

#include "esphome/components/daikin_rotex_uart/Message.h"
#include "esphome/components/daikin_rotex_uart/buffer.h"
#include <vector>

namespace esphome {
namespace daikin_rotex_uart {

class TRequest;

class TMessageManager {
public:
    void add(TMessage const& message);
    bool sendNextRequest(uart::UARTDevice&);
    void handleResponse(uart::UARTDevice&);
private:
    void dumpRequests();
    std::shared_ptr<TRequest> getNextRequestToSend();

    std::vector<TMessage> m_messages;
    TBuffer m_buffer;
};

} // namespace daikin_rotex_uart
} // namespace esphome