//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <memory>
#include <asio/asio.hpp>

#include <config/config.h>
#include "frame_render.h"

class Network {

private:
    asio::io_service _io_service;
    std::unique_ptr<asio::ip::tcp::socket> _pose_socket;
    std::unique_ptr<asio::ip::tcp::socket> _frame_socket;

public:
    explicit Network(const std::string &address);
    
    template<typename ...Args>
    [[nodiscard]] static std::unique_ptr<Network> create(Args &&...args) noexcept {
        try {
            return std::make_unique<Network>(std::forward<Args>(args)...);
        } catch (const asio::system_error &error) {
            std::cerr << "Error: network failure: " << error.what() << std::endl;
            abort();
        }
    }
    
    DisplayState receive_display_state();
    void send_frame(const std::vector<uint8_t> &frame_buffer);
};
