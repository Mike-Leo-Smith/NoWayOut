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
constexpr auto lower_arm_radius = 0.03f;
constexpr auto upper_leg_radius = 0.15f;
constexpr auto lower_leg_radius = 0.10f;
constexpr auto foot_radius = 0.05f;
constexpr auto body_radius = 0.20f;

}

struct GestureNode {
    
    glm::vec3 from;
    glm::vec3 to;
    float radius;
    
    [[nodiscard]] glm::mat4 transform() const noexcept {
        auto direction = to - from;
        auto length = glm::length(direction);
        if (length < 1e-3f) {
            return glm::mat4{1.0f};
        }
        direction *= 1.0f / length;
        return glm::mat4{util::Onb{direction}.inverse_transform()} * glm::translate(glm::mat4{1.0f}, from) * glm::scale(glm::mat4{1.0f}, glm::vec3{radius, length * 0.5f, radius});
    }
};

struct GestureState {
    std::array<GestureNode, 12ul> nodes;
};

struct DisplayState;
struct GameState;

class GestureCapture {

private:
    GestureState _state{};
    GestureState _state_copy;
    mutable std::mutex _mutex;
    std::thread _update_thread;

public:
    explicit GestureCapture(const std::string &address);
    
    [[nodiscard]] const GestureState &state() const noexcept {
        std::lock_guard guard{_mutex};
        _state_copy = _state;
        return _state_copy;
    }
    
    template<typename ...Args>
    [[nodiscard]] static std::unique_ptr<GestureCapture> create(Args &&...args) noexcept {
        return std::make_unique<GestureCapture>(std::forward<Args>(args)...);
    }
};
