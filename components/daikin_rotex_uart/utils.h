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
    static std::string to_hex(std::array<uint8_t, Size> const& data, std::size_t maxlen = Size);
    static std::string to_hex(uint8_t const* data, std::size_t maxlen);
    static std::string to_hex(uint32_t value);
    static std::map<uint16_t, std::string> str_to_map(const std::string& input);

    template<typename... Args>
    static std::string format(const std::string& format, Args... args) {
        int precision = 2;
        bool has_precision = false;

        (void)std::initializer_list<int>{(
            (std::is_floating_point_v<Args> && !has_precision) ?
            (precision = 2, has_precision = true) : 0)...
        };

        std::string format_with_precision = format;
        size_t pos = format_with_precision.find("%.*f");
        if (pos != std::string::npos) {
            format_with_precision.replace(pos, 4, "%.*f");
        }

        const auto size = std::snprintf(nullptr, 0, format_with_precision.c_str(), args...) + 1;
        const auto buffer = std::make_unique<char[]>(size);

        std::snprintf(buffer.get(), size, format_with_precision.c_str(), args...);

        return std::string(buffer.get(), buffer.get() + size - 1);
    }

    static constexpr uint64_t hash_str(std::string_view str) {
        uint64_t hash = 0xcbf29ce484222325;
        for (char c : str) {
            hash = (hash ^ c) * 0x100000001b3;
        }
        return hash;
    }

    static bool equals(float a, float b, float epsilon);
};

inline bool Utils::equals(float a, float b, float epsilon) {
    return std::abs(a - b) <= epsilon;
}

}
}