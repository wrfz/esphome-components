#pragma once

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <variant>
#include <vector>
#include <memory>
#include <map>

namespace esphome {
namespace daikin_rotex_uart {

class Utils {
public:
    template<std::size_t Size>
    static std::string to_hex(std::array<uint8_t, Size> const& data);
};

}
}