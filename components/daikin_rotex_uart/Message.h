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
        int convid,
        int offset,
        int registryID,
        int dataSize,
        int dataType
    );

    //TRequest getRequest();
private:
    std::shared_ptr<TRequest> m_pRequest;
    EntityBase* m_pEntity;
    int m_convid;
    int m_offset;
    int m_registryID;
    int m_dataSize;
    int m_dataType;
};

} // namespace daikin_rotex_uart
} // namespace esphome