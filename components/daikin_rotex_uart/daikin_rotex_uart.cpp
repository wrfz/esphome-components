#include "esphome/components/daikin_rotex_uart/daikin_rotex_uart.h"
#include "esphome/components/daikin_rotex_uart/scheduler.h"
#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/hal.h"

#include <algorithm>
#include <memory>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";
static const std::string TA2 = "outdoor_temp";
static const std::string EXCHANGER_TEMP = "exch_temp";
static const std::string FLOW_RATE = "flow_rate";

DaikinRotexUARTComponent::DaikinRotexUARTComponent()
: m_message_manager()
, m_project_git_hash_sensor(nullptr)
, m_thermal_power_sensor(new UartSensor("thermal_power")) // Create dummy sensors to avoid nullptr without HA api communicaction. Can be overwritten by the user.
, m_thermal_power_raw_sensor(new UartSensor("thermal_power_raw"))
, m_temperature_spread_sensor(new UartSensor("temperature_spread")) // Used to detect valve malfunctions, even if the sensor has not been defined by the user.
, m_temperature_spread_raw_sensor(new UartSensor("temperature_spread_raw"))
, m_project_git_hash()
{
    m_temperature_spread_sensor->set_smooth(true);
}

void DaikinRotexUARTComponent::add_entity(UartSensor* pEntity) {
    add_base_entity(pEntity);
}

void DaikinRotexUARTComponent::add_entity(UartBinarySensor* pEntity) {
    add_base_entity(pEntity);
}

void DaikinRotexUARTComponent::add_entity(UartTextSensor* pEntity) {
    add_base_entity(pEntity);
}

void DaikinRotexUARTComponent::add_base_entity(TEntity* pEntity) {
    m_message_manager.add(pEntity);
    pEntity->set_post_handle([this](TEntity* pEntity, TEntity::TVariant const& current, TEntity::TVariant const& previous){
        on_post_handle(pEntity, current, previous);
    });
}

void DaikinRotexUARTComponent::setup() {
    m_project_git_hash_sensor->publish_state(m_project_git_hash);
}

void DaikinRotexUARTComponent::on_post_handle(TEntity* pEntity, TEntity::TVariant const& current, TEntity::TVariant const& previous) {
    ESP_LOGI(TAG, "on_post_handle()");
    std::list<std::string> const& update_entities = pEntity->get_update_entities();
    for (std::string const& update_entity : update_entities) {
        if (!update_entity.empty()) {
            ESP_LOGI(TAG, "on_post_handle() call_later: %s", update_entity.c_str());
            Scheduler::getInstance().call_later([update_entity, this](){
                updateState(update_entity);
            });
        }
    }
}

void DaikinRotexUARTComponent::loop() {
    m_message_manager.sendNextRequest(*this);

    if (available() > 0) {
        m_message_manager.handleResponse(*this);
    }

    Scheduler::getInstance().update();

    const uint32_t millis = esphome::millis();
    for (TEntity* pEntity : m_message_manager.get_entities()) {
        pEntity->update(millis);
    }
    m_thermal_power_sensor->update(millis);
    m_temperature_spread_sensor->update(millis);
}

void DaikinRotexUARTComponent::updateState(std::string const& id) {
    const uint64_t runtime_hash = Utils::hash_str(id);

    switch (runtime_hash) {
    case Utils::hash_str("temperature_spread"):
        update_temperature_spread();
        break;

    case Utils::hash_str("thermal_power"):
        if (m_message_manager.get_sensor(FLOW_RATE, false) != nullptr) {
            update_thermal_power();
        }
        break;
    }
}

void DaikinRotexUARTComponent::update_thermal_power() {
    ESP_LOGI(TAG, "update_thermal_power()");

    UartSensor const* flow_rate = m_message_manager.get_sensor(FLOW_RATE);
    UartSensor const* tv = m_message_manager.get_sensor("tv");
    UartSensor const* tr = m_message_manager.get_sensor("tr");

    if (flow_rate == nullptr) {
        ESP_LOGE(TAG, "flow_rate is not configured!");
        return;
    }
    if (tv == nullptr) {
        ESP_LOGE(TAG, "tv is not configured!");
        return;
    }
    if (tr == nullptr) {
        ESP_LOGE(TAG, "tr is not configured!");
        return;
    }

    const float thermal_power_raw = (tv->state - tr->state) * (4.19 * flow_rate->state) / 3600.0f;

    ESP_LOGI(TAG, "update_thermal_power() tv: %f, tr: %f, flow: %f, power_raw: %f", tv->state, tr->state, flow_rate->state, thermal_power_raw);

    m_thermal_power_raw_sensor->publish(thermal_power_raw);
    m_thermal_power_sensor->publish(thermal_power_raw);
}

void DaikinRotexUARTComponent::update_temperature_spread() {
    UartSensor const* tv = m_message_manager.get_sensor("tv");
    UartSensor const* tr = m_message_manager.get_sensor("tr");

    if (tv != nullptr && tr != nullptr) {
        const float temperature_spread = tv->state - tr->state;

        ESP_LOGI(TAG, "update_temperature_spread() tv: %f, tr: %f", tv->state, tr->state);

        m_temperature_spread_sensor->publish(temperature_spread);
        m_temperature_spread_raw_sensor->publish(temperature_spread);
    }
}


} // namespace daikin_rotex_uart
} // namespace esphome