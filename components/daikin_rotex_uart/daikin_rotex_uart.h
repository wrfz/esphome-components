#pragma once

#include "esphome/components/daikin_rotex_uart/MessageManager.h"
#include "esphome/components/daikin_rotex_uart/sensors.h"
#include "esphome/components/text_sensor/text_sensor.h"
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

    void set_project_git_hash(text_sensor::TextSensor* pSensor, std::string const& hash);
    void add_entity(UartSensor* pEntity);
    void add_entity(UartBinarySensor* pEntity);
    void add_entity(UartTextSensor* pEntity);

    void call_later(TVoidFunc lambda, uint32_t timeout = 0u) {
        const uint32_t timestamp = millis();
        m_later_calls.push_back({lambda, timestamp + timeout});
    }

private:
    TMessageManager m_message_manager;
    std::list<std::pair<TVoidFunc, uint32_t>> m_later_calls;
    text_sensor::TextSensor* m_project_git_hash_sensor;
    std::string m_project_git_hash;
};

inline void DaikinRotexUARTComponent::set_project_git_hash(text_sensor::TextSensor* pSensor, std::string const& hash) {
    m_project_git_hash_sensor = pSensor;
    m_project_git_hash = hash;
}

} // namespace daikin_rotex_uart
} // namespace esphome