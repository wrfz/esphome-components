#pragma once

#include "esphome/components/daikin_rotex_uart/request.h"
#include <memory>
#include <functional>

namespace esphome {

class EntityBase;

namespace daikin_rotex_uart {

class TMessage {
public:
    enum class Endian {
        Little,
        Big
    };
    using THandleFunc = std::function<uint16_t(uint8_t*)>;

    TMessage(
        std::shared_ptr<TRequest> pRequest,
        EntityBase* pEntity,
        uint8_t registryID,
        uint8_t offset,
        bool isSigned,
        uint8_t dataSize,
        Endian endian,
        double divider,
        THandleFunc handle_lambda,
        bool handle_lambda_set
    );

    std::shared_ptr<TRequest> getRequest() { return m_pRequest; }
    int getRegistryID() const { return m_registryID; }
    uint8_t getOffset() const { return m_offset; }
    uint8_t getDataSize() const { return m_dataSize; }
    EntityBase* getEntity() const { return m_pEntity; }

    void convert(uint8_t* data);
    static uint16_t getUnsignedValue(unsigned char *data, int dataSize, Endian endian);
    static short getSignedValue(unsigned char *data, int datasize, Endian endian);
private:
    std::shared_ptr<TRequest> m_pRequest;
    EntityBase* m_pEntity;
    int m_registryID;
    int m_offset;
    bool m_signed;
    int m_dataSize;
    Endian m_endian;
    double m_divider;
    THandleFunc m_handle_lambda;
    bool m_handle_lambda_set;
};

} // namespace daikin_rotex_uart
} // namespace esphome