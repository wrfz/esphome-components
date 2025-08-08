#include "esphome/components/daikin_rotex_uart/MessageManager.h"
#include "esphome/components/daikin_rotex_uart/request.h"
#include "esphome/components/daikin_rotex_uart/utils.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/hal.h"
#include "esphome/core/entity_base.h"
#include <algorithm>
#include <list>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "daikin-uart";

void TMessageManager::add(TEntity* pEntity) {
    std::shared_ptr<TRequest> pRequest;

    bool found = false;
    for (TEntity* pE : m_messages) {
        if (pE->getRegistryID() == pEntity->getRegistryID()) {
            pEntity->setRequest(pE->getRequest());
            found = true;
            break;
        }
    }
    if (!found) {
        pEntity->setRequest(std::make_shared<TRequest>(pEntity->getRegistryID()));
    }

    m_messages.push_back(pEntity);
}

bool TMessageManager::sendNextRequest(uart::UARTDevice& device) {
    std::shared_ptr<TRequest> pRequest = getNextRequestToSend();
    if (pRequest != nullptr) {
        return pRequest->send(device);
    }
    return false;
}

void TMessageManager::handleResponse(uart::UARTDevice& device) {
    std::string log_message = m_buffer.read(device);

    if (m_buffer.size() >= 2 && m_buffer[0] == 0x15 && m_buffer[1] == 0xEA) {
        ESP_LOGW(TAG, "RX: Invalid request => data: %s", Utils::to_hex(m_buffer.data(), 2).c_str());
        m_buffer.shift(2);
        return;
    }

    if (m_buffer.size() >= 3) {
        if (m_buffer[0] != 0x40) {
            ESP_LOGE(TAG, "Invalid response: %s", Utils::to_hex(m_buffer.data(), m_buffer.size()).c_str());
            m_buffer.clear();
            return;
        }

        const uint8_t registryID = m_buffer[1];
        const uint8_t length = m_buffer[2];

        if (m_buffer.size() >= (2 + length)) {
            const uint8_t header_size = 3;
            for (auto& pEntity : m_messages) {
                if (registryID == pEntity->getRegistryID()) {
                    uint8_t* input = m_buffer.data().data();
                    const uint8_t message_offset = header_size + pEntity->getOffset();
                    if (message_offset < 3 || (message_offset + pEntity->getDataSize()) > m_buffer.size()) {
                        ESP_LOGE(TAG, "RX: Invalid offset! message_offset: %d, message.data_size: %d, buffer.size: %d", message_offset, pEntity->getDataSize(), m_buffer.size());
                        return;
                    }
                    input += message_offset;
                    log_message += "|" + pEntity->convert(input);
                    std::shared_ptr<TRequest> pRequest = pEntity->getRequest();
                    pRequest->setHandled();
                }
            }
            m_buffer.shift(2 + length);
            ESP_LOGI(TAG, "RX: %s", log_message.c_str());
            return;
        }
        ESP_LOGI(TAG, "RX: incomplete buffer: %s", log_message.c_str());
        return;
    }
    ESP_LOGI(TAG, "RX: incomplete header: %s", log_message.c_str());
}

std::shared_ptr<TRequest> TMessageManager::getNextRequestToSend() {
    const uint32_t timestamp = millis();
    static uint32_t last_dump_timestamp = 0;

    if (timestamp > (last_dump_timestamp + 1000)) {
        last_dump_timestamp = timestamp;
        dumpRequests();
    }

    for (auto& message : m_messages) {
        std::shared_ptr<TRequest> pRequest = message->getRequest();
        if (pRequest->isInProgress()) {
            return std::shared_ptr<TRequest>();
        }
    }

    std::shared_ptr<TRequest> pOldestRequest = nullptr;
    for (auto& message : m_messages) {
        std::shared_ptr<TRequest> pRequest = message->getRequest();
        if (pRequest->isRequestRequired()) {
            if (pOldestRequest == nullptr || pRequest->getLastRequestTimestamp() < pOldestRequest->getLastRequestTimestamp()) {
                pOldestRequest = pRequest;
            }
        }
    }
    if (pOldestRequest != nullptr) {
        dumpRequests();
    }
    return pOldestRequest;
}

void TMessageManager::dumpRequests() {
    std::string dump = Utils::format("%d", millis());
    std::list<uint8_t> used;
    bool first = true;
    for (auto& message : m_messages) {
        std::shared_ptr<TRequest> pRequest = message->getRequest();

        const bool contains = (std::find(used.begin(), used.end(), pRequest->getRegistryId()) != used.end());
        if (!contains) {
            used.push_back(pRequest->getRegistryId());
            const char state = pRequest->isInProgress() ? 'P' : (pRequest->isRequestRequired() ? 'R' : '-');
            dump += Utils::format("| %s:%c|%d|%d", Utils::to_hex(pRequest->getRegistryId()).c_str(), state,
                pRequest->getLastRequestTimestamp(), pRequest->getLastResponeTimestamp());
            first = false;
        }
    }
    ESP_LOGI(TAG, "%s", dump.c_str());
}

} // namespace daikin_rotex_uart
} // namespace esphome