//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <vector>
#include <array>
#include <chrono>
#include <functional>
#include <string_view>
#include <asio/asio.hpp>

#include "config.h"

#include "network.h"
#include "frame_render.h"
#include "gesture_capture.h"
#include "game_logic.h"

class Engine {

private:
    std::unique_ptr<Network> _network;
    std::unique_ptr<GameLogic> _game_logic;
    std::unique_ptr<FrameRender> _frame_render;
    std::unique_ptr<GestureCapture> _gesture_capture;
    size_t _frame_count{};
    std::chrono::high_resolution_clock::time_point _last_time_point;

public:
    Engine &set_network(std::unique_ptr<Network> network) noexcept;
    Engine &set_game_logic(std::unique_ptr<GameLogic> game_logic) noexcept;
    Engine &set_frame_render(std::unique_ptr<FrameRender> frame_render) noexcept;
    Engine &set_gesture_capture(std::unique_ptr<GestureCapture> gesture_capture) noexcept;
    void run() noexcept;
};
