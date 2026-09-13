#pragma once

#include "esphome/components/daikin_rotex_uart/sensors.h"
#include "esphome/components/daikin_rotex_uart/buffer.h"
#include "esphome/components/daikin_rotex_uart/protocol.h"
#include <vector>

namespace esphome {
namespace daikin_rotex_uart {

class TRequest;

class TMessageManager {
public:
    void add(TEntity* pEntity);
    bool sendNextRequest(uart::UARTDevice&);
    void handleResponse(uart::UARTDevice&);

    void apply_protocol();

    UartSensor* get_sensor(std::string const& id, bool log_missing = true);
    UartSensor const* get_sensor(std::string const& id, bool log_missing = true) const;

    const TEntity* getEntityById(const std::string& name) const;
    TEntity* getEntityById(const std::string& name);

    const std::vector<TEntity*>& get_entities() const;
private:
    enum class TProbePhase : uint8_t {
        None,
        I,
        S,
    };

    void dumpRequests();
    std::shared_ptr<TRequest> getNextRequestToSend();
    bool send_probe(uart::UARTDevice& device);
    void handleDetectionResponse();
    void resolve_protocol(TProtocol protocol);
    void parseIResponse(std::string const& log_message);
    void parseSResponse(std::string const& log_message);

    std::vector<TEntity*> m_messages;
    TBuffer m_buffer;
    TProbePhase m_probe = TProbePhase::None;
    uint32_t m_probe_sent_at = 0u;
};

inline const std::vector<TEntity*>& TMessageManager::get_entities() const { 
    return m_messages;
}

} // namespace daikin_rotex_uart
} // namespace esphome