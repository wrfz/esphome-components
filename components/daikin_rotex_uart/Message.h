#pragma once

#include "esphome/components/daikin_rotex_uart/request.h"
#include <memory>

namespace esphome {

class EntityBase;

namespace daikin_rotex_uart {

class TMessage {
    enum class Endian {
        Little,
        Big
    };

public:
    TMessage(
        std::shared_ptr<TRequest> pRequest,
        EntityBase* pEntity,
        uint8_t registryID,
        uint8_t offset,
        int convid,
        int dataSize,
        int dataType
    );

    std::shared_ptr<TRequest> getRequest() { return m_pRequest; }
    int getRegistryID() const { return m_registryID; }
    uint8_t getOffset() const { return m_offset; }
    EntityBase* getEntity() const { return m_pEntity; }

    void convert(uint8_t* data);
    static uint16_t getUnsignedValue(unsigned char *data, int dataSize, Endian endian);
    static short getSignedValue(unsigned char *data, int datasize, Endian endian);
private:
    static double convertPress2Temp(double data);

    std::shared_ptr<TRequest> m_pRequest;
    EntityBase* m_pEntity;
    int m_convId;
    int m_offset;
    int m_registryID;
    int m_dataSize;
    int m_dataType;
};

} // namespace daikin_rotex_uart
} // namespace esphome