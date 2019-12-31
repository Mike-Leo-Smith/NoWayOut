//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <cstdint>
#include <array>

namespace util {

constexpr auto u8vec4_to_uint(std::array<uint8_t, 4> b) noexcept {
    return static_cast<uint32_t>(b[0]) << 24u | static_cast<uint32_t>(b[1]) << 16u | static_cast<uint32_t>(b[2]) << 8u | static_cast<uint32_t>(b[3]);
}

constexpr std::array<uint8_t, 4> uint_to_u8vec4(uint32_t u) noexcept {
    return {static_cast<uint8_t>(u >> 24u), static_cast<uint8_t>(u >> 16u), static_cast<uint8_t>(u >> 8u), static_cast<uint8_t>(u)};
}

}