#include "esphome/components/daikin_rotex_uart/pid.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <math.h>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "pid";

float PID::compute(float setpoint, float current_value, float dt, std::string& logstr) {
    if (std::isnan(setpoint) || std::isnan(current_value) || std::isnan(dt)) {
        return 0.f;
    }

    if (std::isinf(setpoint) || std::isinf(current_value) || std::isinf(dt)) {
        return 0.f;
    }

    if (Utils::equals(dt, 0.0f, 0.1f)) {
        return 0.f;
    }

    const float error = setpoint - current_value;

    logstr = Utils::format("sp: %f, cv: %f, e: %f", setpoint, current_value, error);

    // P-Anteil filtern
    m_filtered_p = m_alpha_p * error + (1 - m_alpha_p) * m_filtered_p;
    const float filtered_p = m_filtered_p;
    float p_term = m_p * m_filtered_p;

    // I-Anteil (mit Anti-Windup)
    m_integral += error * dt;
    if (m_integral > m_max_integral) m_integral = m_max_integral;
    if (m_integral < -m_max_integral) m_integral = -m_max_integral;
    float i_term = m_i * m_integral;

    // D-Anteil filtern
    float derivative = (error - m_previous_error) / dt;
    m_filtered_d = m_alpha_d * derivative + (1 - m_alpha_d) * m_filtered_d;
    float d_term = m_d * m_filtered_d;

    // Gesamtausgang
    float output = p_term + i_term + d_term;
    m_previous_error = error;

    m_last_update = esphome::millis();

    logstr += Utils::format(", p: %f, i: %f, d: %f, map: %f, fp: %f, mfp: %f, mp: %f, o: %f",
        p_term, i_term, d_term, m_alpha_p, filtered_p, m_filtered_p, m_p, output);

    return output;
}

}
}