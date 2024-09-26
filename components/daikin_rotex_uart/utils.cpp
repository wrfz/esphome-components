#include "esphome/components/daikin_rotex_uart/utils.h"
#include <regex>

namespace esphome {
namespace daikin_rotex_uart {

static const char* TAG = "Utils";

template<std::size_t Size>
std::string Utils::to_hex(std::array<uint8_t, Size> const& data) {
    std::stringstream str;
    str.setf(std::ios_base::hex, std::ios::basefield);
    str.setf(std::ios_base::uppercase);
    str.fill('0');

    bool first = true;
    for (uint8_t chr : data)
    {
        if (first) {
            first = false;
        } else {
            str << " ";
        }
        str << std::setw(2) << (unsigned short)(std::byte)chr;
    }
    return str.str();
}

template std::string esphome::daikin_rotex_uart::Utils::to_hex<4u>(std::array<unsigned char, 4u> const&);
template std::string esphome::daikin_rotex_uart::Utils::to_hex<64u>(std::array<unsigned char, 64u> const&);

}
}