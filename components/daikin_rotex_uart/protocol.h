#pragma once

#include <cstdint>

namespace esphome {
namespace daikin_rotex_uart {

enum class TProtocol : uint8_t {
    S = 0,
    I = 1,
    AUTO = 2,
};

inline const char* protocol_to_str(TProtocol protocol) {
    switch (protocol) {
        case TProtocol::S: return "S";
        case TProtocol::I: return "I";
        case TProtocol::AUTO: return "AUTO";
    }
    return "UNKNOWN";
}

class TProtocolManager {
private:
    TProtocol m_protocol;

    TProtocolManager() : m_protocol(TProtocol::AUTO) {}
    TProtocolManager(TProtocolManager const&) = delete;
    void operator=(TProtocolManager const&) = delete;

public:
    static TProtocolManager& getInstance() {
        static TProtocolManager instance;
        return instance;
    }

    TProtocol get_active_protocol() const { return m_protocol; }
    bool is_s_active() const { return m_protocol == TProtocol::S; }
    bool is_i_active() const { return m_protocol == TProtocol::I; }
    bool is_auto() const { return m_protocol == TProtocol::AUTO; }
    void set_protocol(TProtocol protocol) { m_protocol = protocol; }
};

} // namespace daikin_rotex_uart
} // namespace esphome
