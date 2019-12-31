//
// Created by Mike Smith on 2019/12/31.
//

#include <zstd.h>
#include <iostream>
#include <turbojpeg.h>
#include <core/util/util.h>
#include "network.h"

DisplayState Network::receive_display_state() {
    std::array<uint8_t, 4> size_buffer{};
    asio::read(*_pose_socket, asio::buffer(size_buffer.data(), 4ul));
    auto compressed_size = util::u8vec4_to_uint(size_buffer);
    constexpr auto max_buffer_size = 4096ul;
    static thread_local uint8_t compressed_buffer[max_buffer_size];
    if (compressed_size > max_buffer_size) {
        std::cerr << "Error: buffer overflow in Network::receive_display_state" << std::endl;
        abort();
    }
    asio::read(*_pose_socket, asio::buffer(compressed_buffer, compressed_size));
    static thread_local char json_string[max_buffer_size];
    ZSTD_decompress(json_string, max_buffer_size, compressed_buffer, compressed_size);
    return DisplayState::from_json_string(json_string);
}

void Network::send_frame(const std::vector<uint8_t> &frame_buffer) {
    
    constexpr auto frame_width = config::eye_frame_width * 2ul;
    constexpr auto frame_height = config::eye_frame_height;
    constexpr auto pixel_size = 3ul;
    constexpr auto frame_buffer_size = frame_width * frame_height * pixel_size;
    
    if (frame_buffer.size() != frame_buffer_size) {
        std::cerr << "Error: frame buffer size mismatch in Network::send_frame" << std::endl;
        abort();
    }
    
    static thread_local uint8_t compressed_frame_buffer[frame_buffer_size];
    static thread_local auto jpeg_handle = tjInitCompress();
    
    auto jpeg_size = 0ul;
    tjCompress(jpeg_handle, const_cast<uint8_t *>(frame_buffer.data()), frame_width, 0, frame_height, pixel_size,
               compressed_frame_buffer + 4ul, &jpeg_size, TJSAMP_444, 70, TJFLAG_FASTDCT);
    *reinterpret_cast<std::array<uint8_t, 4ul> *>(compressed_frame_buffer) = util::uint_to_u8vec4(jpeg_size);
    asio::write(*_frame_socket, asio::buffer(compressed_frame_buffer, jpeg_size + 4ul));
}

Network::Network(const std::string &address) {
    _pose_socket = std::make_unique<asio::ip::tcp::socket>(_io_service);
    _frame_socket = std::make_unique<asio::ip::tcp::socket>(_io_service);
    auto ip_address = asio::ip::address_v4::from_string(address);
    _pose_socket->connect({ip_address, config::pose_socket_port});
    _frame_socket->connect({ip_address, config::frame_socket_port});
}
