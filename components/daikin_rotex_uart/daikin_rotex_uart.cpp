#include "esphome/components/daikin_rotex_uart/daikin_rotex_uart.h"
#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"

#include <algorithm>
#include <memory>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";

DaikinRotexUARTComponent::DaikinRotexUARTComponent()
{
}

void DaikinRotexUARTComponent::setup() {
    std::map<int, std::shared_ptr<TRequest>> map;

    for (auto& entry : m_entities) {
        std::shared_ptr<TRequest> pRequest;

        if (auto const& it = map.find(entry.registryID); it != map.end()) {
            pRequest = it->second;
        } else {
            pRequest = std::make_shared<TRequest>(entry.registryID);
        }

        m_message_manager.add({
            pRequest,
            entry.pEntity,
            entry.convid,
            entry.offset,
            entry.registryID,
            entry.dataSize,
            entry.dataType
        });
    }
}

void DaikinRotexUARTComponent::loop() {
    //ESP_LOGI(TAG, "loop");

    m_message_manager.sendNextRequest(this);

    std::array<uint8_t, 64> buffer {0};
    uint32_t max_to_read = available();
    if (max_to_read > 0) {
        const auto to_read = std::min(max_to_read, sizeof(buffer));
        read_array(buffer.data(), to_read);

        ESP_LOGI(TAG, "read n: %d, buffer: %s", to_read, Utils::to_hex(buffer).c_str());
    }
}

} // namespace daikin_rotex_uart
} // namespace esphome