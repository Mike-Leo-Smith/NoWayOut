//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <memory>
#include <thread>
#include <string>
#include <array>
#include <mutex>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <util/util.h>

namespace config {

constexpr auto upper_arm_radius = 0.05f;
constexpr auto lower_arm_radius = 0.05f;
constexpr auto upper_leg_radius = 0.05f;
constexpr auto lower_leg_radius = 0.05f;
constexpr auto foot_radius = 0.05f;
constexpr auto body_radius = 0.10f;
constexpr auto head_radius = 0.15f;
constexpr auto neck_radius = 0.05f;

}

struct GestureNode {
    
    glm::vec3 from;
    glm::vec3 to;
    float radius;
    
    [[nodiscard]] glm::mat4 transform() const noexcept {
        
        auto bad_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, -10.0f, 0.0f});
        if ((from.x == 0.0f && from.y == 0.0f && from.z == 0.0f) || (to.x == 0.0f && to.y == 0.0f && to.z == 0.0f)) {
            return bad_transform;
        }
        
        auto direction = to - from;
        auto length = glm::length(direction);
        if (length < 1e-3f) {
            return bad_transform;
        }
        direction *= 1.0f / length;
        glm::vec3 offset{0.0f, 1.2f, 0.0f};
        return glm::translate(glm::mat4{1.0f}, from + offset) * glm::mat4{util::Onb{direction}.inverse_transform()} * glm::scale(glm::mat4{1.0f}, glm::vec3{radius, length * 0.5f, radius});
    }
};

struct GestureState {
    std::array<GestureNode, 12ul> nodes;
    GestureNode neck;
};

struct DisplayState;
struct GameState;

class GestureCapture {

private:
    GestureState _state{};
    mutable GestureState _state_copy{};
    mutable std::mutex _mutex;
    std::thread _update_thread;

public:
    explicit GestureCapture(std::string address);
    
    [[nodiscard]] const GestureState &state() const noexcept {
        std::lock_guard guard{_mutex};
        constexpr auto alpha = 0.8f;
        for (auto i = 0ul; i < _state.nodes.size(); i++) {
            _state_copy.nodes[i].from = (1.0f - alpha) * _state_copy.nodes[i].from + alpha * _state.nodes[i].from;
            _state_copy.nodes[i].to = (1.0f - alpha) * _state_copy.nodes[i].to + alpha * _state.nodes[i].to;
            _state_copy.nodes[i].radius = (1.0f - alpha) * _state_copy.nodes[i].radius + alpha * _state.nodes[i].radius;
        }
        _state_copy.neck.from = (1.0f - alpha) * _state_copy.neck.from + alpha * _state.neck.from;
        _state_copy.neck.to = (1.0f - alpha) * _state_copy.neck.to + alpha * _state.neck.to;
        _state_copy.neck.radius = (1.0f - alpha) * _state_copy.neck.radius + alpha * _state.neck.radius;
        return _state_copy;
    }
    
    template<typename ...Args>
    [[nodiscard]] static std::unique_ptr<GestureCapture> create(Args &&...args) noexcept {
        return std::make_unique<GestureCapture>(std::forward<Args>(args)...);
    }
};
