#include "esphome/components/daikin_rotex_uart/BidiMap.h"
#include "esphome/core/log.h"

#include <limits>

namespace esphome {
namespace daikin_rotex_uart {

BidiMap::Iterator BidiMap::findNextByKey(uint16_t value) const {
    if (key_to_value.empty()) {
        return key_to_value.end();
    }

    Iterator closest = key_to_value.end();
    float closest_distance = std::numeric_limits<float>::max();

    for (auto it = key_to_value.begin(); it != key_to_value.end(); ++it) {
        float distance = std::abs(it->first - value);

        if (distance < closest_distance) {
            closest_distance = distance;
            closest = it;
        }
    }

    return closest;
}

}
}