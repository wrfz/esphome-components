#include "esphome/components/daikin_rotex_uart/daikin_rotex_uart.h"
#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"

#include <iostream>
#include <algorithm>
#include <memory>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";

static int my_log_output(const char *fmt, va_list args) {
    return vprintf(fmt, args);
}

DaikinRotexUARTComponent::DaikinRotexUARTComponent()
{
    std::cout << "DaikinRotexUARTComponent::DaikinRotexUARTComponent" << std::endl;

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_set_vprintf(my_log_output);

    ESP_LOGI("aaa", "DaikinRotexUARTComponent");
}

void DaikinRotexUARTComponent::add_entity(EntityBase* pEntityBase) {
    if (TEntity* pEntity = dynamic_cast<TEntity*>(pEntityBase)) {
        m_message_manager.add(pEntity);
    }
}

void DaikinRotexUARTComponent::setup() {
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