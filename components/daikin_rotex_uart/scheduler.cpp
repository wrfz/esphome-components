#include "esphome/components/daikin_rotex_uart/scheduler.h"
#include "esphome/core/hal.h"

#include <algorithm>

namespace esphome {
namespace daikin_rotex_uart {

CallHandle::CallHandle()
: m_id(0)
, m_scheduler(nullptr) {
}

CallHandle::CallHandle(CallID id, Scheduler* scheduler)
: m_id(id)
, m_scheduler(scheduler) {
}

bool CallHandle::cancel() {
    if (!is_valid()) return false;
    bool result = m_scheduler->cancel_call(m_id);
    if (result) {
        m_id = 0;
    }
    return result;
}

CallHandle Scheduler::call_later(CallHandle::TVoidFunc lambda, uint32_t timeout) {
    CallHandle::CallID new_id = m_next_call_id.fetch_add(1);

    m_later_calls.push_back({
        std::move(lambda),
        esphome::millis() + timeout,
        new_id
    });

    return CallHandle(new_id, this);
}

bool Scheduler::is_call_valid(CallHandle::CallID id) const {
    if (id == 0) return false;

    for (const auto& call : m_later_calls) {
        if (call.id == id) {
            return true;
        }
    }
    return false;
}

bool Scheduler::accelerate_call(CallHandle::CallID id) {
    for (auto& call : m_later_calls) {
        if (call.id == id) {
            call.execution_time = esphome::millis();
            return true;
        }
    }
    return false;
}

bool Scheduler::cancel_call(CallHandle::CallID id) {
    auto it = std::remove_if(m_later_calls.begin(), m_later_calls.end(),
        [id](const DelayedCall& call) {
            return call.id == id;
        });

    if (it != m_later_calls.end()) {
        m_later_calls.erase(it, m_later_calls.end());
        return true;
    }
    return false;
}

void Scheduler::update() {
    if (m_later_calls.empty()) return;

    const uint32_t current_time = esphome::millis();

    auto it = m_later_calls.begin();
    while (it != m_later_calls.end()) {
        if ((int32_t)(current_time - it->execution_time) >= 0) {
            it->function();
            it = m_later_calls.erase(it);
        } else {
            ++it;
        }
    }
}

}
}
