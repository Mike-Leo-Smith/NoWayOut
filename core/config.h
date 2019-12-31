//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <cstdint>

namespace config {
    constexpr size_t eye_frame_width = 512ul;
    constexpr size_t eye_frame_height = 512ul;
    constexpr uint16_t pose_socket_port = 23333u;
    constexpr uint16_t frame_socket_port = 24444u;
};
