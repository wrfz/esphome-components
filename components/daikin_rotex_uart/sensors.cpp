#include "esphome/components/daikin_rotex_uart/sensors.h"
#include "esphome/components/daikin_rotex_uart/entity.h"

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";

/////////////////////// UartSensor ///////////////////////

bool UartSensor::handleValue(uint16_t value, TEntity::TVariant& current) {
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

/////////////////////// UartTextSensor ///////////////////////

bool UartTextSensor::handleValue(uint16_t value, TEntity::TVariant& current) {
    const auto it = m_map.findByKey(value);
    current = it != m_map.end() ? it->second : Utils::format("INVALID<%d>", value);
    publish_state(std::get<std::string>(current));
    return true;
}

/////////////////////// UartBinarySensor ///////////////////////

bool UartBinarySensor::handleValue(uint16_t value, TEntity::TVariant& current) {
    current = value > 0;
    publish_state(std::get<bool>(current));
    return true;
}

}
}