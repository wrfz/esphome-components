#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/components/daikin_rotex_uart/utils.h"

#include "esphome/components/binary_sensor/binary_sensor.h" // temporary
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/sensor/sensor.h"

#include "esphome/core/hal.h"

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";

std::string TEntity::convert(uint8_t* data) {
    uint16_t value = 0u;
    if (m_config.handle_lambda_set)
    {
        value = m_config.handle_lambda(data);
    }
    else
    {
        if (m_config.data_size == 1)
        {
            value = (uint16_t)data[0];
        }
        else if (m_config.endian == Endian::Little)
        {
            value = ((uint16_t)(data[1] << 8) | (uint16_t)data[0]);
        }
        else // Big Endian
        {
            value = ((uint16_t)(data[0] << 8) | (uint16_t)data[1]);
        }
    }

    TEntity::TVariant current;
    const bool value_valid = handleValue(value, current);

    if (value_valid) {
        std::string value;
        if (std::holds_alternative<double>(current)) {
            value = Utils::format("%.*f", m_config.accuracy_decimals, std::get<double>(current));
        } else if (std::holds_alternative<bool>(current)) {
            value = std::get<bool>(current);
        } else if (std::holds_alternative<std::string>(current)) {
            value = std::get<std::string>(current);
        } else {
            value = "Unsupported value type!";
        }
        return Utils::format("%s: %s", m_config.name.c_str(), value.c_str());
    }
    return "INV";
}

}
}