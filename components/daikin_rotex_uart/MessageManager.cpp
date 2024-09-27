#include "esphome/components/daikin_rotex_uart/MessageManager.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/hal.h"
#include "esphome/core/entity_base.h"
#include <algorithm>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "message_manager";

void TMessageManager::add(TMessage const& message) {
    m_messages.push_back(message);
}

bool TMessageManager::sendNextRequest(uart::UARTDevice& device) {
    std::shared_ptr<TRequest> pRequest = getNextRequestToSend();
    if (pRequest != nullptr) {
        return pRequest->send(device);
    }
    return false;
}

void TMessageManager::handleResponse(uart::UARTDevice& device) {
    std::array<uint8_t, 64> buffer {0};
    const auto to_read = std::min(static_cast<uint32_t>(device.available()), sizeof(buffer));
    device.read_array(buffer.data(), to_read);

    ESP_LOGI(TAG, "read n: %d, buffer: %s", to_read, Utils::to_hex(buffer).c_str());

    uint8_t registryID = buffer[1];
    uint8_t offset = 3;
    for (auto& message : m_messages) {
        if (registryID == message.getRegistryID()) {
            //ESP_LOGE(TAG, "handle: registryID: %d", registryID);
            uint8_t* input = buffer.data();
            input += message.getOffset() + offset;
            message.convert(input);
            std::shared_ptr<TRequest> pRequest = message.getRequest();
            pRequest->setHandled();
        }
    }
}

std::shared_ptr<TRequest> TMessageManager::getNextRequestToSend() {
    const uint32_t timestamp = millis();

    for (auto& message : m_messages) {
        std::shared_ptr<TRequest> pRequest = message.getRequest();
        //ESP_LOGE(TAG, "name: %s, inProgress: %d", message.getEntity()->get_name().c_str(), pRequest->isInProgress());
        if (pRequest->isInProgress()) {
            return std::shared_ptr<TRequest>();
        }
    }

    for (auto& message : m_messages) {
        std::shared_ptr<TRequest> pRequest = message.getRequest();
        //ESP_LOGE(TAG, "name: %s, isRequestRequired: %d", message.getEntity()->get_name().c_str(), pRequest->isRequestRequired());
        if (pRequest->isRequestRequired()) {
            return pRequest;
        }
    }
    return std::shared_ptr<TRequest>();
}

} // namespace daikin_rotex_uart
} // namespace esphome