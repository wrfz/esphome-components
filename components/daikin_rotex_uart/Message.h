#pragma once

#include "esphome/components/daikin_rotex_uart/request.h"
#include <memory>

namespace esphome {

class EntityBase;

namespace daikin_rotex_uart {

class TMessage {
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
    static unsigned short getUnsignedValue(unsigned char *data, int dataSize, int cnvflg);
    static short getSignedValue(unsigned char *data, int datasize, int cnvflg);
private:
    static void convertTable300(unsigned char *data, int tableID, char *ret);
    static void convertTable203(unsigned char *data, char *ret);
    static void convertTable204(unsigned char *data, char *ret);
    static double convertTable312(unsigned char *data);
    static void convertTable315(unsigned char *data, char *ret);
    static void convertTable316(unsigned char *data, char *ret);
    static void convertTable200(unsigned char *data, char *ret);
    static void convertTable217(unsigned char *data, char *ret);
    static double convertPress2Temp(double data);

    std::shared_ptr<TRequest> m_pRequest;
    EntityBase* m_pEntity;
    int m_convId;
    int m_offset;
    int m_registryID;
    int m_dataSize;
    int m_dataType;
    char m_asString[30];
};

} // namespace daikin_rotex_uart
} // namespace esphome