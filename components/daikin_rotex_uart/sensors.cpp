#include "esphome/components/daikin_rotex_uart/sensors.h"
#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";

/////////////////////// UartSensor ///////////////////////

UartSensor::UartSensor()
: m_state(std::numeric_limits<float>::quiet_NaN())
, m_range()
, m_pid(0.2, 0.05f, 0.05f, 0.2, 0.2, 0.1f)
, m_smooth(false)
, m_smooth_state(std::numeric_limits<float>::quiet_NaN())
{}

UartSensor::UartSensor(std::string const& id)
: UartSensor()
{
    m_config.id = id;
}

bool UartSensor::handleValue(uint16_t value, TEntity::TVariant& current, TVariant& previous) {
    if (m_config.isSigned) {
        current = static_cast<int16_t>(value) / m_config.divider;
    } else {
        current = value / m_config.divider;
    }

    const float float_value = std::get<double>(current);
    const bool valid = !m_range.required() || (float_value >= m_range.min && float_value <= m_range.max);
    if (valid) {
        publish_state(float_value);
    } else {
        ESP_LOGE(TAG, "UartSensor::handleValue() => Sensor<%s> value<%f> out of range[%f, %f]",
            get_name().c_str(), float_value, m_range.min, m_range.max);
    }

    return valid;
}

void UartSensor::update(uint32_t millis) {
    TEntity::update(millis);

    if (m_smooth) {
        const float dt = (static_cast<float>(esphome::millis()) - m_pid.get_last_update()) / 1000.0f; // seconds
        if (dt > 10.0f) {
            if (std::isnan(m_smooth_state)) {
                m_smooth_state = m_state;
            }

            std::string logstr;
            m_smooth_state += m_pid.compute(m_state, m_smooth_state, dt, logstr);
            //ESP_LOGI(TAG, "update() PID %s: %s, val: %f", get_id().c_str(), logstr.c_str(), m_smooth_state);

            m_smooth_state = std::ceil(m_smooth_state * 100.0) / 100.0;

            publish_state(m_smooth_state);
        }
    }
}

void UartSensor::publish(float state) {
    m_state = state;
    if (!m_smooth) {
        publish_state(state);
    }
}

/////////////////////// UartTextSensor ///////////////////////

bool UartTextSensor::handleValue(uint16_t value, TEntity::TVariant& current, TVariant& previous) {
    const auto it = m_map.findByKey(value);
    current = it != m_map.end() ? it->second : Utils::format("INVALID<%d>", value);
    publish_state(std::get<std::string>(current));
    return true;
}

/////////////////////// UartBinarySensor ///////////////////////

bool UartBinarySensor::handleValue(uint16_t value, TEntity::TVariant& current, TVariant& previous) {
    current = value > 0;
    publish_state(std::get<bool>(current));
    return true;
}

}
}