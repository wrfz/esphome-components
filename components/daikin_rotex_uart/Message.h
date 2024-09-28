#pragma once

#include "esphome/components/daikin_rotex_uart/request.h"
#include <memory>

namespace esphome {

class EntityBase;

namespace daikin_rotex_uart {

class TMessage {
public:
    enum class Endian {
        Little,
        Big
    };

    TMessage(
        std::shared_ptr<TRequest> pRequest,
        EntityBase* pEntity,
        uint8_t registryID,
        uint8_t offset,
        bool isSigned,
        int dataSize,
        Endian endian,
        double divider
    );

    std::shared_ptr<TRequest> getRequest() { return m_pRequest; }
    int getRegistryID() const { return m_registryID; }
    uint8_t getOffset() const { return m_offset; }
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
};

} // namespace daikin_rotex_uart
} // namespace esphome