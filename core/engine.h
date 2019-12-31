//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <vector>
#include <array>
#include <functional>
#include <string_view>
#include <asio/asio.hpp>

#include "config.h"

#include "frame_render.h"
#include "gesture_capture.h"
#include "game_logic.h"

class Engine {

private:
    asio::io_service _io_service;
    asio::ip::tcp::socket _pose_socket;
    asio::ip::tcp::socket _frame_socket;
    
public:
    void connect(std::string_view address);
    void loop();
};
