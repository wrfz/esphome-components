#pragma once

#include "esphome/components/daikin_rotex_uart/utils.h"
#include <string>

namespace esphome {
namespace daikin_rotex_uart {

class PID {
public:
    PID(float p, float i, float d, float max_integral, float alpha_p, float alpha_d)
    : m_p(p)
    , m_i(i)
    , m_d(d)
    , m_previous_error(0.0f)
    , m_integral(0.0f)
    , m_max_integral(max_integral)
    , m_alpha_p(alpha_p)
    , m_alpha_d(alpha_d)
    , m_filtered_p(0)
    , m_filtered_d(0)
    , m_last_update(0)
    , m_logging(false)
    {
    }

    float compute(float setpoint, float current_value, float dt, std::string& logstr);

    uint32_t get_last_update() const { return m_last_update; }

    void set_logging(bool logging) { m_logging = logging; }

private:
    float m_p;
    float m_i;
    float m_d;
    float m_previous_error;
    float m_integral;
    float m_max_integral;
    float m_alpha_p;
    float m_alpha_d;
    float m_filtered_p;
    float m_filtered_d;
    uint32_t m_last_update;
    bool m_logging;
};

}
}