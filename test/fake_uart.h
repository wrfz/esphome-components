#pragma once

#include "esphome/components/uart/uart.h"

#include <deque>
#include <vector>

namespace esphome::test {

/// UART bus for the tests: bytes "received" are queued with receive(), bytes written are kept in tx.
class FakeUart : public uart::UARTComponent {
public:
    std::vector<uint8_t> tx;

    void receive(std::vector<uint8_t> const& data) { m_rx.insert(m_rx.end(), data.begin(), data.end()); }

    void write_array(const uint8_t* data, size_t len) override { tx.insert(tx.end(), data, data + len); }
    bool peek_byte(uint8_t* data) override {
        if (m_rx.empty()) return false;
        *data = m_rx.front();
        return true;
    }
    bool read_array(uint8_t* data, size_t len) override {
        if (len > m_rx.size()) return false;
        for (size_t i = 0; i < len; ++i) {
            data[i] = m_rx.front();
            m_rx.pop_front();
        }
        return true;
    }
    size_t available() override { return m_rx.size(); }
    uart::UARTFlushResult flush() override { return uart::UARTFlushResult::UART_FLUSH_RESULT_SUCCESS; }

protected:
    void check_logger_conflict() override {}

private:
    std::deque<uint8_t> m_rx;
};

/// UARTDevice on a FakeUart (the component itself is a UARTDevice).
class FakeDevice : public uart::UARTDevice {
public:
    explicit FakeDevice(FakeUart* parent) : uart::UARTDevice(parent) {}
};

} // namespace esphome::test
