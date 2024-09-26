#include "esphome/components/daikin_rotex_uart/MessageManager.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace daikin_rotex_uart {

void TMessageManager::add(TMessage const& message) {
    m_messages.push_back(message);
}

void TMessageManager::sendNextRequest(uart::UARTDevice& device) {
    std::shared_ptr<TRequest> pRequest = getNextRequestToSend();
    if (pRequest != nullptr) {
        return pRequest->send(device);
    }
    return false;


    for (auto& message : m_messages) {
        m_message();
    }
}

std::shared_ptr<TRequest> TMessageManager::getNextRequestToSend() {
    const uint32_t timestamp = millis();

    for (auto& message : m_messages) {
        std::shared_ptr<TRequest> pRequest = message.getRequest();
        if (pRequest->isGetInProgress()) {
            return std::shared_ptr<TRequest>;
        }
    }

    for (auto& request : m_requests) {
        std::shared_ptr<TRequest> pRequest = message.getRequest();
        if (request.isGetNeeded()) {
            return &request;
        }
    }
    return std::shared_ptr<TRequest>;
}


} // namespace daikin_rotex_uart
} // namespace esphome