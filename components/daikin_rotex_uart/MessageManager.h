#pragma once

#include "esphome/components/daikin_rotex_uart/sensors.h"
#include "esphome/components/daikin_rotex_uart/buffer.h"
#include <vector>

namespace esphome {
namespace daikin_rotex_uart {

class TRequest;

class TMessageManager {
public:
    void add(TEntity* pEntity);
    bool sendNextRequest(uart::UARTDevice&);
    void handleResponse(uart::UARTDevice&);

    UartSensor* get_sensor(std::string const& id);
    UartSensor const* get_sensor(std::string const& id) const;

    const TEntity* getEntityById(const std::string& name) const;
    TEntity* getEntityById(const std::string& name);
private:
    void dumpRequests();
    std::shared_ptr<TRequest> getNextRequestToSend();

    std::vector<TEntity*> m_messages;
    TBuffer m_buffer;
};

} // namespace daikin_rotex_uart
} // namespace esphome