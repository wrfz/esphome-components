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

void TBuffer::read(uart::UARTDevice& device) {
    const auto to_read = std::min(static_cast<uint32_t>(device.available()), m_buffer.size() - m_size);
    uint8_t* data = m_buffer.data() + m_size;
    bool ok = false;
    if (device.read_array(data, to_read)) {
        m_size += to_read;
        ok = true;
    }
    ESP_LOGI(TAG, "RX: %s", Utils::to_hex(data, to_read).c_str());
    //ESP_LOGI("receive", "buffer: %s", Utils::to_hex(m_buffer.data(), m_size).c_str());
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
