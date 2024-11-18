#include "esphome/components/daikin_rotex_uart/Message.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/log.h"

#include <limits>
#include <cstring>
#include <cmath>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin-uart";

TMessage::TMessage(
    std::shared_ptr<TRequest> pRequest,
    EntityBase* pEntity,
    uint8_t registryID,
    uint8_t offset,
    bool isSigned,
    uint8_t dataSize,
    Endian endian,
    double divider,
    THandleFunc handle_lambda,
    bool handle_lambda_set
)
: m_pRequest(pRequest)
, m_pEntity(pEntity)
, m_registryID(registryID)
, m_offset(offset)
, m_signed(isSigned)
, m_dataSize(dataSize)
, m_endian(endian)
, m_divider(divider)
, m_handle_lambda(handle_lambda)
, m_handle_lambda_set(handle_lambda_set)
{
}

void TMessage::convert(uint8_t* data) {
    double value = 0.0;
    bool value_valid = false;
    if (m_handle_lambda_set)
    {
        value = m_handle_lambda(data);
        value_valid = true;
    }
    else
    {
        double dblData = m_signed ?
            getSignedValue(data, m_dataSize, m_endian) :
            getUnsignedValue(data, m_dataSize, m_endian);

        dblData /= m_divider;

        if (!std::isnan(dblData))
        {
            value = dblData;
            value_valid = true;
        }
    }

    if (value_valid) {
        std::string str = Utils::format("%d", value);
        ESP_LOGI(TAG, "%s: %s", m_pEntity->get_name().c_str(), str.c_str());
        if (sensor::Sensor* pSensor = dynamic_cast<sensor::Sensor*>(m_pEntity)) {
            pSensor->publish_state(value);
        } else if (binary_sensor::BinarySensor* pSensor = dynamic_cast<binary_sensor::BinarySensor*>(m_pEntity)) {
            pSensor->publish_state(value);
        }
    }
}

uint16_t TMessage::getUnsignedValue(unsigned char *data, int dataSize, Endian endian) {
    uint16_t result;
    if (dataSize == 1)
    {
        result = (uint16_t)data[0];
    }
    else if (endian == Endian::Little)
    {
        result = ((uint16_t)(data[1] << 8) | (uint16_t)data[0]);
    }
    else // Big Endian
    {
        result = ((uint16_t)(data[0] << 8) | (uint16_t)data[1]);
    }
    return result;
}

short TMessage::getSignedValue(unsigned char *data, int datasize, Endian endian) {
    uint16_t num = getUnsignedValue(data, datasize, endian);
    short result = (short)num;
    if ((num & 0x8000) != 0)
    {
        num = ~num;
        num += 1;
        result = (short)((int)num * -1);
    }
    return result;
}

} // namespace daikin_rotex_uart
} // namespace esphome