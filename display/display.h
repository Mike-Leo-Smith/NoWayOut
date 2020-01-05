//
// Created by Mike Smith on 2019/12/19.
// Copyright (c) 2019 Mike Smith. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <opencv2/opencv.hpp>
#include <asio/asio.hpp>

#include <core/graphics/shader.h>
#include <util/util.h>

#include "display_state.h"

class Display {

public:
    static constexpr auto max_transmission_error = 10ul;

private:
    asio::io_service _io_service;
    asio::ip::tcp::socket _pose_socket{_io_service};
    asio::ip::tcp::socket _frame_socket{_io_service};
    size_t _transmission_error_count{0ul};
    std::unique_ptr<Shader> _display_shader;
    std::vector<uint8_t> _frame_buffer;
    cv::Mat _frame;
    uint32_t _vertex_buffer_handle{};
    uint32_t _vertex_array_handle{};
    uint32_t _texture_handle{};
    std::string _ip_address;
    std::mutex _mutex;
    std::thread _decode_thread;
    std::condition_variable _cv;
    DisplayState _head_pose{};
    bool _decode_in_progress{false};
    
    [[nodiscard]] static std::string _get_ip_address() noexcept;
    [[noreturn]] void _decode_loop() noexcept;
    bool _async_connect() noexcept;
    
    template<typename F>
    bool _with_sockets(F &&do_something) noexcept {
        
        if (_transmission_error_count >= max_transmission_error) {
            if (!(_ip_address = _get_ip_address()).empty() && _async_connect()) {
                _transmission_error_count = 0ul;
            }
            return false;
        }
        
        try {
            do_something(_pose_socket, _frame_socket);
            return true;
        } catch (const std::system_error &e) {
            std::cout << "Transmission error: " << e.what() << std::endl;
            _transmission_error_count++;
            _ip_address = _get_ip_address();
            return false;
        }
    }

public:
    static std::unique_ptr<Display> create() noexcept { return std::make_unique<Display>(); }
    
    void initialize();
    void draw(DisplayState state, uint32_t width, uint32_t height);
};
