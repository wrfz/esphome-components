#include "esphome/components/daikin_rotex_uart/daikin_rotex_uart.h"
#include "esphome/components/daikin_rotex_uart/manifest_action.h"
#include "esphome/components/daikin_rotex_uart/scheduler.h"
#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/hal.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";
static const std::string TA2 = "outdoor_temp";
static const std::string EXCHANGER_TEMP = "exch_temp";
static const std::string FLOW_RATE = "flow_rate";
static const std::string SATURATION_TEMP = "saturation_temp";  // from the pressure sensor between 4-way valve and condenser
static const std::string LIQUID_TEMP = "t_liq";           // refrigerant leaving the condenser (plate heat exchanger)
static const std::string DISCHARGE_TEMP = "temp_after_compr";
static const std::string COMPRESSOR_SPEED = "inv_freq";
static const std::string FOUR_WAY_VALVE = "4_way_valve";
static const uint16_t FOUR_WAY_VALVE_HEATING = 0x01;     // key of "heating" in the map of 4_way_valve

DaikinRotexUARTComponent::DaikinRotexUARTComponent()
: m_message_manager()
, m_project_git_hash_sensor(nullptr)
, m_thermal_power_sensor(new UartSensor("thermal_power")) // Create dummy sensors to avoid nullptr without HA api communicaction. Can be overwritten by the user.
, m_thermal_power_raw_sensor(new UartSensor("thermal_power_raw"))
, m_temperature_spread_sensor(new UartSensor("temperature_spread")) // Used to detect valve malfunctions, even if the sensor has not been defined by the user.
, m_temperature_spread_raw_sensor(new UartSensor("temperature_spread_raw"))
, m_subcooling_sensor(new UartSensor("subcooling"))
, m_discharge_superheat_sensor(new UartSensor("discharge_superheat"))
, m_protocol_from_config(TProtocol::AUTO)
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
#ifdef USE_DAIKIN_ROTEX_UART_MANIFEST
    // Registered here, after the generated code has set up the YAML actions
    // (initialize_user_services() replaces the list, register_user_service() appends).
    if (!m_manifest_pages.empty() && api::global_api_server != nullptr) {
        api::global_api_server->register_user_service(new ManifestAction(m_manifest_action_name, m_manifest_pages));
        ESP_LOGI(TAG, "manifest action '%s' registered (%u page(s))",
            m_manifest_action_name, static_cast<unsigned>(m_manifest_pages.size()));
    }
#endif

    if (m_protocol_from_config != TProtocol::AUTO) {
        TProtocolManager::getInstance().set_protocol(m_protocol_from_config);
    }
    m_message_manager.apply_protocol();

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

    case Utils::hash_str("refrigerant_circuit"):
        update_refrigerant_circuit();
        break;

    case Utils::hash_str("thermal_power"):
        UartSensor const* flow_rate = m_message_manager.get_sensor(FLOW_RATE, false);
        if (flow_rate != nullptr && flow_rate->is_active()) {
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

// The pressure sensor sits between the 4-way valve and the condenser: only while heating with the
// compressor running it measures the high pressure of the condenser. Otherwise (defrost, cooling,
// standstill) subcooling and superheat have no meaning. An entity the user did not configure is not checked.
bool DaikinRotexUARTComponent::heating_with_compressor() const {
    UartSensor const* compressor = m_message_manager.get_sensor(COMPRESSOR_SPEED, false);
    if (compressor != nullptr && !(compressor->state > 0.0f)) {
        return false;
    }
    TEntity const* valve = m_message_manager.getEntityById(FOUR_WAY_VALVE);
    if (valve != nullptr && !static_cast<UartTextSensor const*>(valve)->is_state_of(FOUR_WAY_VALVE_HEATING)) {
        return false;
    }
    return true;
}

// subcooling: saturation temperature (from the pressure) - liquid temperature at the condenser outlet
// discharge superheat: discharge temperature - saturation temperature
// Without a value of an input or outside heating with running compressor both are NaN (unknown in HA).
void DaikinRotexUARTComponent::update_refrigerant_circuit() {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    UartSensor const* saturation = m_message_manager.get_sensor(SATURATION_TEMP, false);
    UartSensor const* liquid = m_message_manager.get_sensor(LIQUID_TEMP, false);
    UartSensor const* discharge = m_message_manager.get_sensor(DISCHARGE_TEMP, false);

    const bool valid = saturation != nullptr && heating_with_compressor();
    const float subcooling = valid && liquid != nullptr ? saturation->state - liquid->state : nan;
    const float superheat = valid && discharge != nullptr ? discharge->state - saturation->state : nan;

    ESP_LOGD(TAG, "update_refrigerant_circuit() subcooling: %f, discharge superheat: %f", subcooling, superheat);

    m_subcooling_sensor->publish(subcooling);
    m_discharge_superheat_sensor->publish(superheat);
}


} // namespace daikin_rotex_uart
} // namespace esphome