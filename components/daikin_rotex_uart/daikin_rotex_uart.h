#pragma once

#include "esphome/components/daikin_rotex_uart/MessageManager.h"
#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include <list>

namespace esphome {

class EntityBase;

namespace uart {
    class UARTComponent;
}

namespace daikin_rotex_uart {

class DaikinRotexUARTComponent: public Component, public uart::UARTDevice {
 public:
    using TVoidFunc = std::function<void()>;

    DaikinRotexUARTComponent();
    void setup() override;
    void loop() override;

    void add_entity(EntityBase* pEntityBase);

    void call_later(TVoidFunc lambda, uint32_t timeout = 0u) {
        const uint32_t timestamp = millis();
        m_later_calls.push_back({lambda, timestamp + timeout});
    }

private:
    TMessageManager m_message_manager;
    std::list<std::pair<TVoidFunc, uint32_t>> m_later_calls;
};

} // namespace daikin_rotex_uart
} // namespace esphome