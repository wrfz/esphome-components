#pragma once

namespace esphome {
namespace daikin_rotex_uart {

class TRequest {
public:
    TRequest(int registryID)
    : m_registryID(registryID)
    {

    }

    voif send();
private:
    int m_registryID;
};

} // namespace daikin_rotex_uart
} // namespace esphome