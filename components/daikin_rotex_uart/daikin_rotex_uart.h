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
    void set_thermal_power_sensor(UartSensor* pSensor);
    void set_thermal_power_sensor_raw(UartSensor* pSensor);

    void add_entity(UartSensor* pEntity);
    void add_entity(UartBinarySensor* pEntity);
    void add_entity(UartTextSensor* pEntity);

private:
    void add_base_entity(TEntity* pEntity);

    void on_post_handle(TEntity* pEntity, TEntity::TVariant const& current, TEntity::TVariant const& previous);

    void updateState(std::string const& id);
    void update_thermal_power();

    TMessageManager m_message_manager;
    text_sensor::TextSensor* m_project_git_hash_sensor;

    UartSensor* m_thermal_power_sensor;
    UartSensor* m_thermal_power_raw_sensor;
    
    std::string m_project_git_hash;
};

inline void DaikinRotexUARTComponent::set_project_git_hash(text_sensor::TextSensor* pSensor, std::string const& hash) {
    m_project_git_hash_sensor = pSensor;
    m_project_git_hash = hash;
}

inline void DaikinRotexUARTComponent::set_thermal_power_sensor(UartSensor* pSensor) {
    m_thermal_power_sensor = pSensor;
    pSensor->set_smooth(true);
}

inline void DaikinRotexUARTComponent::set_thermal_power_sensor_raw(UartSensor* pSensor) {
    m_thermal_power_raw_sensor = pSensor;
}

} // namespace daikin_rotex_uart
} // namespace esphome