#pragma once

#include "esphome/components/daikin_rotex_uart/Message.h"
#include <vector>

namespace esphome {
namespace daikin_rotex_uart {

class TRequest;

class TMessageManager {
public:
    void add(TMessage const& message);
    void sendNextRequest(uart::UARTDevice&);
private:
    std::shared_ptr<TRequest> getNextRequestToSend();

    std::vector<TMessage> m_messages;
};

} // namespace daikin_rotex_uart
} // namespace esphome