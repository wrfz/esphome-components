#include "esphome/components/boot_logger/boot_logger.h"
#include "esphome/components/logger/logger.h"
#include "esphome/core/util.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace boot_logger {

static const char* TAG = "boot_logger";

BootLogger::BootLogger(std::uint32_t heap_limit, std::uint32_t dump_lines_per_loop, std::uint32_t connect_delay)
: m_config{heap_limit, dump_lines_per_loop, connect_delay} {
}

void BootLogger::setup() {
    ESP_LOGI(TAG, "setup");
    if (logger::global_logger != nullptr) {
        logger::global_logger->add_on_log_callback([this](int level, const char* tag, const char* message) {
            if (m_state == State::BUFFERING) {
                m_items.emplace_back(LogItem{level, std::string(tag), std::string(message)});
            }
        });
    }
    m_setup_ts = esphome::millis();
}

void BootLogger::loop() {
    const bool connected = esphome::api_is_connected();

    const uint32_t dump_ts = m_connected_ts + m_config.connect_delay;
    bool log_connected = false;
    if (connected && m_connected_ts == 0) {
        m_connected_ts = esphome::millis();
        log_connected = true;

        ESP_LOGE(TAG, "state: %d, connected: %d, free_heap: %d, size: %d, setup: %d, connected_ts: %d, dump_ts: %d, millis: %d",
            m_state, connected, esp_get_free_heap_size(), m_items.size(), m_setup_ts, m_connected_ts, dump_ts, esphome::millis());
    }

    if ((m_connected_ts > 0 && esphome::millis() > dump_ts) || (esp_get_free_heap_size() < m_config.heap_limit)) {
        m_state = State::DUMPING;
    }

    if (m_state == State::DUMPING) {
        dumping();
    }
}

void BootLogger::dumping() {
    uint32_t index = 0;
    while (!m_items.empty() && index < m_config.dump_lines_per_loop) {
        const auto& item = m_items.front();
        switch (item.level) {
        case ESPHOME_LOG_LEVEL_ERROR:
            ESP_LOGE("STARTUP", item.message.c_str());
            break;
        case ESPHOME_LOG_LEVEL_WARN:
            ESP_LOGW("STARTUP", item.message.c_str());
            break;
        case ESPHOME_LOG_LEVEL_INFO:
            ESP_LOGI("STARTUP", item.message.c_str());
            break;
            case ESPHOME_LOG_LEVEL_DEBUG:
            ESP_LOGD("STARTUP", item.message.c_str());
            break;
        case ESPHOME_LOG_LEVEL_VERBOSE:
            ESP_LOGV("STARTUP", item.message.c_str());
            break;
        }
        m_items.pop_front();
        ++index;
    }
}

}
}