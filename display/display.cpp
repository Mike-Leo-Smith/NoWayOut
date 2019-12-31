//
// Created by Mike Smith on 2019/12/19.
// Copyright (c) 2019 Mike Smith. All rights reserved.
//

#include <iostream>
#include <string_view>
#include <zstd.h>

#include <config/config.h>
#include "display.h"

void Display::initialize() {
    
    using namespace std::string_literals;
    
    _display_shader = Shader::create(
        R"(#version 410 core
layout (location = 0) in vec2 aCoord;
out vec2 TexCoord;
void main() {
    TexCoord = vec2(aCoord.x, 1.0f - aCoord.y);
    gl_Position = vec4((2.0f * aCoord - 1.0f).xy, 0.0f, 1.0f);
})"s,
        R"(#version 410 core
precision highp float;
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screen;
void main() {
    FragColor = vec4(texture(screen, TexCoord).rgb, 1.0f);
})"s);
    
    _frame.create(config::eye_frame_height, config::eye_frame_width * 2ul, CV_8UC3);
    
    constexpr float vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f};
    
    glGenBuffers(1, &_vertex_buffer_handle);
    glBindBuffer(GL_ARRAY_BUFFER, _vertex_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glGenVertexArrays(1, &_vertex_array_handle);
    glBindVertexArray(_vertex_array_handle);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, sizeof(float) * 2ul, nullptr);
    
    glGenTextures(1, &_texture_handle);
    glBindTexture(GL_TEXTURE_2D, _texture_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, config::eye_frame_width * 2ul, config::eye_frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    _decode_thread = std::thread{[this] { _decode_loop(); }};
}

std::string Display::_get_ip_address() noexcept {
    return "127.0.0.1";
}

void Display::draw(DisplayState state, uint32_t width, uint32_t height) {
    
    {
        std::lock_guard lock_guard{_mutex};  // wait for decode...
        
        glBindTexture(GL_TEXTURE_2D, _texture_handle);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, config::eye_frame_width * 2ul, config::eye_frame_height, GL_RGB, GL_UNSIGNED_BYTE, _frame.data);
        
        _head_pose = state;
        _decode_in_progress = true;
    }
    _cv.notify_one();
    
    // display
    glViewport(0u, 0u, width, height);
    glScissor(0u, 0u, width, height);
    glDisable(GL_DEPTH_TEST);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture_handle);
    
    glBindVertexArray(_vertex_array_handle);
    _display_shader->with([](auto &shader) {
        shader["frame"] = 0;
        glDrawArrays(GL_TRIANGLES, 0, 6);
    });
    
}

void Display::_decode_loop() noexcept {
    
    for (;;) {
        std::unique_lock lock{_mutex};
        _cv.wait(lock, [this] { return _decode_in_progress; });
        
        if (!_with_sockets([this](asio::ip::tcp::socket &pose_socket, asio::ip::tcp::socket &frame_socket) {
            
            // send head pose
            auto json_string = _head_pose.to_json_string();
            static thread_local char json_buffer[4096ul];
            auto send_size = ZSTD_compress(json_buffer + 4ul, 4092ul, json_string.data(), json_string.size() + 1ul, 2);
            *reinterpret_cast<std::array<uint8_t, 4ul> *>(json_buffer) = util::uint_to_u8vec4(static_cast<uint32_t>(send_size));
            asio::write(pose_socket, asio::buffer(json_buffer, send_size + 4ul));
            
            // receive frame
            std::array<uint8_t, 4> length_buffer;
            asio::read(frame_socket, asio::buffer(length_buffer.data(), 4ul));
            _frame_buffer.resize(util::u8vec4_to_uint(length_buffer));
            asio::read(frame_socket, asio::buffer(_frame_buffer.data(), _frame_buffer.size()));
            
            // decompress image
            cv::imdecode(_frame_buffer, cv::IMREAD_UNCHANGED, &_frame);
            
        })) {  // failure, clear frame and write error message
            _frame = cv::Scalar::all(0);
            if (_ip_address.empty()) {
                cv::putText(_frame, "Network Failure", cv::Point{80, 176}, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar::all(255), 1, cv::LINE_AA);
                if (_head_pose.mode != DisplayMode::CENTER) {
                    cv::putText(_frame, "Network Failure", cv::Point{48 + config::eye_frame_width, 176}, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar::all(255), 1, cv::LINE_AA);
                }
            } else {
                cv::putText(_frame, "Waiting...", cv::Point{80, 176}, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar::all(255), 1, cv::LINE_AA);
                cv::putText(_frame, "IP: " + _ip_address, cv::Point{80, 224}, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar::all(255), 1, cv::LINE_AA);
                if (_head_pose.mode != DisplayMode::CENTER) {
                    cv::putText(_frame, "Waiting...", cv::Point{48 + config::eye_frame_width, 176}, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar::all(255), 1, cv::LINE_AA);
                    cv::putText(_frame, "IP: " + _ip_address, cv::Point{48 + config::eye_frame_width, 224}, cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar::all(255), 1, cv::LINE_AA);
                }
            }
        }
        _decode_in_progress = false;
    }
}

bool Display::_async_connect() noexcept {
    bool pose_socket_success = false;
    bool frame_socket_success = false;
    auto address = asio::ip::address_v4::from_string(_ip_address);
    _io_service.reset();
    _pose_socket = asio::ip::tcp::socket{_io_service};
    _frame_socket = asio::ip::tcp::socket{_io_service};
    asio::ip::tcp::acceptor pose_socket_acceptor{_io_service, {address, config::pose_socket_port}};
    asio::ip::tcp::acceptor frame_socket_acceptor{_io_service, {address, config::frame_socket_port}};
    pose_socket_acceptor.async_accept(_pose_socket, [&](const asio::error_code &error) { pose_socket_success = !error; });
    frame_socket_acceptor.async_accept(_frame_socket, [&](const asio::error_code &error) { frame_socket_success = !error; });
    using namespace std::chrono_literals;
    _io_service.run_for(200ms);
    pose_socket_acceptor.close();
    frame_socket_acceptor.close();
    return pose_socket_success && frame_socket_success;
}
