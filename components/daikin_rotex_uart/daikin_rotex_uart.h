#pragma once

#include "esphome/components/daikin_rotex_uart/MessageManager.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include <list>

namespace esphome {

class EntityBase;

namespace uart {
    class UARTComponent;
}

namespace daikin_rotex_uart {

class DaikinRotexUARTComponent: public Component, public uart::UARTDevice {
    using Endian = TMessage::Endian;

    struct TEntityArguments {
        EntityBase* pEntity;
        std::string name;
        uint8_t registryID;
        uint8_t offset;
        bool isSigned;
        uint8_t dataSize;
        Endian endian;
        double divider;
        uint8_t accuracy_decimals;
        TMessage::THandleFunc handle_lambda;
        bool handle_lambda_set;
        bool forward_to_can;
    };

    using TEntityArgumentsList = std::list<TEntityArguments>;

 public:
    using TVoidFunc = std::function<void()>;

    DaikinRotexUARTComponent();
    void setup() override;
    void loop() override;

    void set_can_id(std::string const& can_id) { m_can_id = can_id; }
    void set_entity(TEntityArguments const& arg) { m_entities.push_back(arg); }

    void call_later(TVoidFunc lambda, uint32_t timeout = 0u) {
        const uint32_t timestamp = millis();
        m_later_calls.push_back({lambda, timestamp + timeout});
    }

private:
    std::string m_can_id;
    TEntityArgumentsList m_entities;
    TMessageManager m_message_manager;
    std::list<std::pair<TVoidFunc, uint32_t>> m_later_calls;
};

} // namespace daikin_rotex_uart
} // namespace esphome