#pragma once
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace daikin_rotex_uart {

class TBuffer {
    using BufferType = std::array<uint8_t, 64>;
public:
    TBuffer();
    std::string read(uart::UARTDevice& device);
    uint8_t size() const;
    const BufferType& data() const;
    BufferType& data();
    uint8_t& operator[](uint8_t offset);
    void shift(uint8_t length);
    void clear();
private:
    BufferType m_buffer;
    uint8_t m_size;
};

inline uint8_t TBuffer::size() const {
    return m_size;
}

inline const TBuffer::BufferType& TBuffer::data() const {
    return m_buffer;
}

inline TBuffer::BufferType& TBuffer::data() {
    return m_buffer;
}

inline uint8_t& TBuffer::operator[](uint8_t offset) {
    return m_buffer[offset];
}

}
}
