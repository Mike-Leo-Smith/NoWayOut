//
// Created by Mike Smith on 2019/12/31.
//

#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <asio/asio.hpp>
#include "gesture_capture.h"

GestureCapture::GestureCapture(std::string address) {
    
    _update_thread = std::thread{[ip_address = std::move(address), this] {

        asio::io_service io_service;
        asio::ip::tcp::socket socket{io_service};
        socket.connect({asio::ip::address_v4::from_string(ip_address), 6666});

        std::cout << "GestureCapture connected!" << std::endl;

        char buffer[1024ul];
        glm::vec3 coords[25ul];
        
        glm::vec3 offset{0.0f, 1.0f, 0.0f};

        for (;;) {

            asio::read(socket, asio::buffer(buffer, 1024ul));
            std::istringstream ss{buffer};
            for (auto &&coord : coords) {
                ss >> coord.x >> coord.y >> coord.z;
                coord += offset;
            }

            std::lock_guard guard{_mutex};

            _state.nodes[0] = {coords[2], coords[3], config::upper_arm_radius};  // left upper arm
            _state.nodes[1] = {coords[3], coords[4], config::lower_arm_radius};  // left lower arm
            _state.nodes[2] = {coords[5], coords[6], config::upper_arm_radius};  // right upper arm
            _state.nodes[3] = {coords[6], coords[7], config::lower_arm_radius};  // right lower arm

            _state.nodes[4] = {coords[9], coords[10], config::upper_leg_radius};  // left upper leg
            _state.nodes[5] = {coords[10], coords[11], config::lower_leg_radius};  // left lower leg
            _state.nodes[6] = {coords[5], coords[6], config::upper_leg_radius};  // right upper leg
            _state.nodes[7] = {coords[6], coords[7], config::lower_leg_radius};  // right lower leg

            _state.nodes[8] = {coords[24], coords[23], config::foot_radius};  // left foot
            _state.nodes[9] = {coords[21], coords[20], config::foot_radius};  // left foot

            _state.nodes[10] = {coords[1], coords[8], config::body_radius};  // body

            _state.nodes[11] = {coords[17], coords[18], glm::length(coords[17] - coords[18]) * 0.5f};  // head
            
            std::cout << "keypoint_0: " << coords[0].x << " " << coords[0].y << " " << coords[0].z << std::endl;

        }

    }};
    
}
