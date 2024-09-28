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

        bool found = false;

        if (auto const& it = map.find(entry.registryID); it != map.end()) {
            pRequest = it->second;
            found = true;
        } else {
            pRequest = std::make_shared<TRequest>(entry.registryID);
        }

        m_message_manager.add({
            pRequest,
            entry.pEntity,
            entry.registryID,
            entry.offset,
            entry.isSigned,
            entry.dataSize,
            entry.endian,
            entry.divider
        });
    }
}

void DaikinRotexUARTComponent::loop() {
    m_message_manager.sendNextRequest(*this);

    if (available() > 0) {
        m_message_manager.handleResponse(*this);
    }

    for (auto it = m_later_calls.begin(); it != m_later_calls.end(); ) {
        if (millis() > it->second) {
            it->first();
            it = m_later_calls.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace daikin_rotex_uart
} // namespace esphome