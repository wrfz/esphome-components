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
    struct TEntityArguments {
        EntityBase* pEntity;
        int convid;
        int offset;
        int registryID;
        int dataSize;
        int dataType;
    };

    using TEntityArgumentsList = std::list<TEntityArguments>;

    /*class LabelDef
    {
    public:
        int convid;
        int offset;
        int registryID;
        int dataSize;
        int dataType;
        const char *label;
        char *data;
        char asString[30];

        LabelDef(){}
        LabelDef(int registryIDp, int offsetp, int convidp, int dataSizep, int dataTypep, const char *labelp):
            convid(convidp), offset(offsetp), registryID(registryIDp), dataSize(dataSizep), dataType(dataTypep), label(labelp) {}
    };*/

public:
    DaikinRotexUARTComponent();
    void setup() override;
    void loop() override;

    void set_entity(std::string const& name, TEntityArguments const& arg) { m_entities.push_back(arg); }

    //void set_uart(esphome::uart::UARTComponent* pUartComponent) { m_pUartComponent = pUartComponent; }
    //void set_component_source(std::string const& source) {}
private:
    TEntityArgumentsList m_entities;
    TMessageManager m_message_manager;
};

} // namespace daikin_rotex_uart
} // namespace esphome