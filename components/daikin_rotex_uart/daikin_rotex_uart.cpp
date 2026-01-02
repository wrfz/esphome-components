#include "esphome/components/daikin_rotex_uart/daikin_rotex_uart.h"
#include "esphome/components/daikin_rotex_uart/entity.h"
#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"

#include <algorithm>
#include <memory>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin_rotex_uart";
static const std::string TA2 = "outdoor_temp";
static const std::string EXCHANGER_TEMP = "exch_temp";

DaikinRotexUARTComponent::DaikinRotexUARTComponent()
: m_message_manager()
, m_later_calls()
, m_project_git_hash_sensor(nullptr)
, m_project_git_hash()
{
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
}

void DaikinRotexUARTComponent::loop() {
    m_message_manager.sendNextRequest(*this);

    if (available() > 0) {
        m_message_manager.handleResponse(*this);
    }

    for (auto it = m_later_calls.begin(); it != m_later_calls.end(); ) {
        if (millis() > it->second) {
            it->first();
            it = m_later_calls.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace daikin_rotex_uart
} // namespace esphome