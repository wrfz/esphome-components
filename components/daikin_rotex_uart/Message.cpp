#include "esphome/components/daikin_rotex_uart/Message.h"

namespace esphome {
namespace daikin_rotex_uart {

TMessage::TMessage(
    std::shared_ptr<TRequest> pRequest,
    EntityBase* pEntity,
    int convid,
    int offset,
    int registryID,
    int dataSize,
    int dataType
) {

}

} // namespace daikin_rotex_uart
} // namespace esphome