#include "esphome/components/daikin_rotex_uart/buffer.h"
#include "esphome/components/daikin_rotex_uart/utils.h"

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin-uart";

TBuffer::TBuffer()
: m_buffer {0}
, m_size(0u)
{
}

std::string TBuffer::read(uart::UARTDevice& device) {
    const auto to_read = std::min(static_cast<size_t>(device.available()), m_buffer.size() - m_size);
    uint8_t* data = m_buffer.data() + m_size;
    if (device.read_array(data, to_read)) {
        m_size += to_read;
    }
    return Utils::to_hex(data, to_read);
}

void TBuffer::shift(uint8_t length) {
    std::memmove(m_buffer.data(), m_buffer.data() + length, m_size - length);
    m_size -= length;

    //ESP_LOGW("TBuffer", "shift: %d, m_size: %d", length, m_size);
}

void TBuffer::clear() {
    m_size = 0;
    //ESP_LOGW("TBuffer", "clear: m_size: %d", m_size);
}


}
}
