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

static const uint32_t PROBE_TIMEOUT_MS = 2000u;

// Fixed S-response frame length (registry + content + crc). 0x53/0x54/0x55 => 18 bytes,
// 0x50/0x56 => 6 bytes.
static uint8_t s_frame_length(uint8_t registryID) {
    switch (registryID) {
        case 0x50:
        case 0x56:
            return 6;
        case 0x53:
        case 0x54:
        case 0x55:
        default:
            return 18;
    }
}

void TMessageManager::add(TEntity* pEntity) {
    m_messages.push_back(pEntity);
}

// (Re)build the shared request objects according to the currently active protocol.
// Entities of the inactive protocol (and any entity before the protocol is resolved
// during auto-detection) get a null request and are skipped everywhere.
void TMessageManager::apply_protocol() {
    for (TEntity* pEntity : m_messages) {
        pEntity->setRequest(nullptr);
    }

    for (TEntity* pEntity : m_messages) {
        if (!pEntity->is_active()) {
            continue;
        }

        std::shared_ptr<TRequest> pRequest = nullptr;
        for (TEntity* pEntity2 : m_messages) {
            if (pEntity2 == pEntity || !pEntity2->is_active() || pEntity2->getRequest() == nullptr) {
                continue;
            }
            if (pEntity2->getRegistryID() == pEntity->getRegistryID()) {
                pRequest = pEntity2->getRequest();
                break;
            }
        }

        if (pRequest == nullptr) {
            pRequest = std::make_shared<TRequest>(TProtocolManager::getInstance().get_active_protocol(), pEntity->getRegistryID());
            pEntity->setRequest(pRequest);
        } else {
            pEntity->setRequest(pRequest);
        }
    }
}

void TMessageManager::resolve_protocol(TProtocol protocol) {
    ESP_LOGI(TAG, "Protocol resolved: %s", protocol_to_str(protocol));
    TProtocolManager::getInstance().set_protocol(protocol);
    m_probe = TProbePhase::None;
    m_buffer.clear();
    apply_protocol();
}

// Send the next auto-detection probe. Probe I (0x60) first, on timeout probe the
// S-registry 0x53, on a second timeout fall back to the I-protocol.
bool TMessageManager::send_probe(uart::UARTDevice& device) {
    const uint32_t timestamp = millis();

    if (m_probe == TProbePhase::None) {
        m_probe = TProbePhase::I;
        m_probe_sent_at = timestamp;
        return TRequest::sendFrame(device, TProtocol::I, 0x60);
    }

    if (m_probe == TProbePhase::I && (timestamp - m_probe_sent_at) > PROBE_TIMEOUT_MS) {
        m_probe = TProbePhase::S;
        m_probe_sent_at = timestamp;
        return TRequest::sendFrame(device, TProtocol::S, 0x53);
    }

    if (m_probe == TProbePhase::S && (timestamp - m_probe_sent_at) > PROBE_TIMEOUT_MS) {
        m_probe = TProbePhase::None;
        resolve_protocol(TProtocol::I);
        return false;
    }

    return false;
}

const TEntity* TMessageManager::getEntityById(const std::string& id) const {
    for (auto pEntity : m_messages) {
        if (pEntity->get_id() == id) {
            return pEntity;
        }
    }
    return nullptr;
}

TEntity* TMessageManager::getEntityById(const std::string& id) {
    return const_cast<TEntity*>(static_cast<const TMessageManager*>(this)->getEntityById(id));
}

UartSensor* TMessageManager::get_sensor(std::string const& id, bool log_missing) {
    TEntity* pEntity = getEntityById(id);
    if (UartSensor* pSensor = static_cast<UartSensor*>(pEntity)) {
        return pSensor;
    } else if (log_missing) {
        if (pEntity) {
            ESP_LOGE(TAG, "Entity is not a sensor: %s", pEntity->get_id().c_str());
        } else {
            ESP_LOGE(TAG, "Entity not found: %s", id.c_str());
        }
    }
    return nullptr;
}

UartSensor const* TMessageManager::get_sensor(std::string const& id, bool log_missing) const {
    TEntity const* pEntity = getEntityById(id);
    if (UartSensor const* pSensor = static_cast<UartSensor const*>(pEntity)) {
        return pSensor;
    } else if (log_missing) {
        if (pEntity) {
            ESP_LOGE(TAG, "Const Entity is not a sensor: %s", pEntity->get_id().c_str());
        } else {
            ESP_LOGE(TAG, "Const Entity not found: %s", id.c_str());
        }
    }
    return nullptr;
}

bool TMessageManager::sendNextRequest(uart::UARTDevice& device) {
    if (TProtocolManager::getInstance().is_auto()) {
        return send_probe(device);
    }

    std::shared_ptr<TRequest> pRequest = getNextRequestToSend();
    if (pRequest != nullptr) {
        return pRequest->send(device);
    }
    return false;
}

void TMessageManager::handleResponse(uart::UARTDevice& device) {
    std::string log_message = m_buffer.read(device);

    if (TProtocolManager::getInstance().is_auto()) {
        handleDetectionResponse();
        return;
    }

    if (m_buffer.size() >= 2 && m_buffer[0] == 0x15 && m_buffer[1] == 0xEA) {
        ESP_LOGW(TAG, "RX: Invalid request => data: %s", Utils::to_hex(m_buffer.data(), 2).c_str());
        m_buffer.shift(2);
        return;
    }

    if (TProtocolManager::getInstance().is_s_active()) {
        parseSResponse(log_message);
    } else {
        parseIResponse(log_message);
    }
}

void TMessageManager::handleDetectionResponse() {
    if (m_probe == TProbePhase::None) {
        m_buffer.clear();
        return;
    }

    if (m_buffer.size() >= 2 && m_buffer[0] == 0x15 && m_buffer[1] == 0xEA) {
        // Device rejected the probe frame => it speaks the other protocol.
        m_buffer.shift(2);
        resolve_protocol(m_probe == TProbePhase::I ? TProtocol::S : TProtocol::I);
        return;
    }

    if (m_buffer.size() >= 3 && m_buffer[0] == 0x40) {
        // Legacy I-protocol response header.
        m_buffer.clear();
        resolve_protocol(TProtocol::I);
        return;
    }

    if (m_probe == TProbePhase::S) {
        const uint8_t registryID = m_buffer[0];
        if (registryID == 0x50 || registryID == 0x53 || registryID == 0x54 || registryID == 0x55 || registryID == 0x56) {
            m_buffer.clear();
            resolve_protocol(TProtocol::S);
            return;
        }
    }
}

void TMessageManager::parseIResponse(std::string const& log_message) {
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
            std::string msg = log_message;
            for (auto& pEntity : m_messages) {
                if (!pEntity->is_active()) {
                    continue;
                }
                if (registryID == pEntity->getRegistryID()) {
                    uint8_t* input = m_buffer.data().data();
                    const uint8_t message_offset = header_size + pEntity->getOffset();
                    if (message_offset < 3 || (message_offset + pEntity->getDataSize()) > m_buffer.size()) {
                        ESP_LOGE(TAG, "RX: Invalid offset! message_offset: %d, message.data_size: %d, buffer.size: %d", message_offset, pEntity->getDataSize(), m_buffer.size());
                        return;
                    }
                    input += message_offset;
                    msg += "|" + pEntity->convert(input);
                    pEntity->getRequest()->setHandled();
                }
            }
            m_buffer.shift(2 + length);
            ESP_LOGI(TAG, "RX: %s", msg.c_str());
            return;
        }
        ESP_LOGI(TAG, "RX: incomplete buffer (%d of %d): %s", static_cast<int>(m_buffer.size()), static_cast<int>(2 + length), Utils::to_hex(m_buffer.data(), m_buffer.size()).c_str());
        return;
    }
    ESP_LOGI(TAG, "RX: incomplete header: %s", log_message.c_str());
}

void TMessageManager::parseSResponse(std::string const& log_message) {
    if (m_buffer.size() < 1) {
        ESP_LOGI(TAG, "RX[S]: incomplete header: %s", log_message.c_str());
        return;
    }

    const uint8_t registryID = m_buffer[0];
    const uint8_t frame_length = s_frame_length(registryID);

    if (m_buffer.size() < frame_length) {
        ESP_LOGI(TAG, "RX[S]: incomplete buffer (%d of %d): %s", static_cast<int>(m_buffer.size()), static_cast<int>(frame_length), Utils::to_hex(m_buffer.data(), m_buffer.size()).c_str());
        return;
    }

    // S-response CRC check. Warn only - never drop a frame on mismatch.
    const uint8_t crc = TRequest::getCRC(m_buffer.data().data(), frame_length - 1);
    if (crc != m_buffer[frame_length - 1]) {
        ESP_LOGW(TAG, "RX[S] CRC mismatch: expected %02X, got %02X", crc, m_buffer[frame_length - 1]);
    }

    const uint8_t header_size = 1;
    std::string msg = log_message;
    for (auto& pEntity : m_messages) {
        if (!pEntity->is_active()) {
            continue;
        }
        if (registryID == pEntity->getRegistryID()) {
            uint8_t* input = m_buffer.data().data();
            const uint8_t message_offset = header_size + pEntity->getOffset();
            if (message_offset < 1 || (message_offset + pEntity->getDataSize()) > frame_length) {
                ESP_LOGE(TAG, "RX[S]: Invalid offset! message_offset: %d, message.data_size: %d, frame.size: %d", message_offset, pEntity->getDataSize(), frame_length);
                return;
            }
            input += message_offset;
            msg += "|" + pEntity->convert(input);
            pEntity->getRequest()->setHandled();
        }
    }
    m_buffer.shift(frame_length);
    ESP_LOGI(TAG, "RX[S]: %s", msg.c_str());
}

std::shared_ptr<TRequest> TMessageManager::getNextRequestToSend() {
    const uint32_t timestamp = millis();
    static uint32_t last_dump_timestamp = 0;

    if (timestamp > (last_dump_timestamp + 1000)) {
        last_dump_timestamp = timestamp;
        dumpRequests();
    }

    for (auto& message : m_messages) {
        if (!message->is_active() || message->getRequest() == nullptr) {
            continue;
        }
        std::shared_ptr<TRequest> pRequest = message->getRequest();
        if (pRequest->isInProgress()) {
            return std::shared_ptr<TRequest>();
        }
    }

    std::shared_ptr<TRequest> pOldestRequest = nullptr;
    for (auto& message : m_messages) {
        if (!message->is_active() || message->getRequest() == nullptr) {
            continue;
        }
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
        if (!message->is_active() || message->getRequest() == nullptr) {
            continue;
        }
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
    ESP_LOGI(TAG, "count: %d, %s", m_messages.size(), dump.c_str());
}

} // namespace daikin_rotex_uart
} // namespace esphome