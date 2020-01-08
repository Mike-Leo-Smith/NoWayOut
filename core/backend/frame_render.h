//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <vector>
#include <string_view>
#include <glm/glm.hpp>

#include <core/graphics/framebuffer.h>
#include <core/graphics/shader.h>
#include <core/graphics/geometry.h>

#include <opencv2/opencv.hpp>
#include <GLFW/glfw3.h>

#include <config/config.h>
#include <display/display_state.h>

struct GameState;

class FrameRender {

private:
    std::vector<uint8_t> _pixel_buffer;
    std::unique_ptr<Framebuffer> _framebuffer;
    std::unique_ptr<Shader> _shader;
    std::unique_ptr<Shader> _shadow_shader;
    std::unique_ptr<Geometry> _ground;
    GLFWwindow *_context{nullptr};
    float _start_time{0.0f};
    uint32_t _shadow_fbo_handle;
    uint32_t _shadow_texture_handle;
    glm::vec3 _light_direction{1.0f, 1.0f, 1.0f};
    glm::vec3 _light_emission{1.0f, 1.0f, 1.0f};
    glm::mat4 _light_transform{1.0f};

private:
    void _create_shadow_map();
    void _render(const GameState &game_state, glm::mat4 view_matrix, glm::mat4 projection_matrix);
    void _shadow_pass(const GameState &game_state);

public:
    FrameRender();
    [[nodiscard]] const std::vector<uint8_t> &frame() const noexcept { return _pixel_buffer; }
    void update(const GameState &game_state, const DisplayState &display_state);
    
    template<typename ...Args>
    [[nodiscard]] static std::unique_ptr<FrameRender> create(Args &&...args) noexcept {
        return std::make_unique<FrameRender>(std::forward<Args>(args)...);
    }
};
