#pragma once

#include "esphome/components/daikin_rotex_uart/BidiMap.h"
#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace daikin_rotex_uart {

/////////////////////// UartSensor ///////////////////////

class UartSensor : public sensor::Sensor, public TEntity {
    struct Range {
        float min;
        float max;

        bool required() { return min != 0 && max != 0; }
    };

public:
    UartSensor() = default;
    void set_range(Range const& range) { m_range = range; }
protected:
    virtual bool handleValue(uint16_t value, TVariant& current) override;
private:
    Range m_range;
    uint32_t trys;
};

/////////////////////// UartTextSensor ///////////////////////

class UartTextSensor : public text_sensor::TextSensor, public TEntity {
public:
    UartTextSensor() = default;
    void set_map(std::string const& str_map) { m_map = Utils::str_to_map(str_map); }
protected:
    virtual bool handleValue(uint16_t value, TVariant& current) override;
private:
    BidiMap m_map;
};

/////////////////////// UartBinarySensor ///////////////////////

class UartBinarySensor : public binary_sensor::BinarySensor, public TEntity {
public:
    UartBinarySensor() = default;

protected:
    virtual bool handleValue(uint16_t value, TVariant& current) override;
};

}  // namespace ld2410
}  // namespace esphome
