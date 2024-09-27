#include "esphome/components/daikin_rotex_uart/utils.h"
#include <regex>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "Utils";

template<std::size_t Size>
std::string Utils::to_hex(std::array<uint8_t, Size> const& data, std::size_t maxlen) {
    std::ostringstream oss;
    size_t length = std::min(maxlen, data.size());

    for (size_t i = 0; i < length; ++i) {
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
        if (i != length - 1) {
            oss << " ";
        }
    }

    return oss.str();
}

template std::string Utils::to_hex<64u>(std::array<uint8_t, 64u> const&, std::size_t);
template std::string Utils::to_hex<4u>(std::array<unsigned char, 4u> const&, std::size_t);

}
}