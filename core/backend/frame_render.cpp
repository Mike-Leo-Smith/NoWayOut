//
// Created by Mike Smith on 2019/12/31.
//

#include "frame_render.h"
#include "game_logic.h"

void FrameRender::update(const GameState &game_state, const DisplayState &display_state) {
    
    _framebuffer->with([&](auto &framebuffer) {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glViewport(0, 0, config::eye_frame_width * 2ul, config::eye_frame_height);
        glScissor(0, 0, config::eye_frame_width * 2ul, config::eye_frame_height);
        glClearColor(1.0f, 0.5f, 0.25f, 1.0f);
        glClear(static_cast<uint32_t>(GL_COLOR_BUFFER_BIT) | static_cast<uint32_t>(GL_DEPTH_BUFFER_BIT));
        glm::mat4 head_transform{1.0f};
        for (auto &&organ : game_state.organs) {
            if (organ.organ_type == organ_type_t::PLAYER_HEAD) {
                auto origin = organ.obj->getWorldTransform().getOrigin();
                head_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{origin.x(), -origin.y(), origin.z()});
                break;
            }
        }
        if (display_state.mode == DisplayMode::CENTER) {
            _render(game_state, display_state.time, display_state.view_matrix[0] * head_transform, display_state.projection_matrix[0]);
        } else {
            glViewport(0, 0, config::eye_frame_width, config::eye_frame_height);
            glScissor(0, 0, config::eye_frame_width, config::eye_frame_height);
            _render(game_state, display_state.time, display_state.view_matrix[0] * head_transform, display_state.projection_matrix[0]);
            glViewport(config::eye_frame_width, 0, config::eye_frame_width, config::eye_frame_height);
            glScissor(config::eye_frame_width, 0, config::eye_frame_width, config::eye_frame_height);
            _render(game_state, display_state.time, display_state.view_matrix[1] * head_transform, display_state.projection_matrix[1]);
        }
    });
    _framebuffer->copy_pixels(_pixel_buffer);
}

FrameRender::FrameRender()
    : _pixel_buffer(config::eye_frame_width * 2ul * config::eye_frame_height * 3ul) {
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#endif
    
    _window = glfwCreateWindow(config::eye_frame_width * 2ul, config::eye_frame_height, "Backend", nullptr, nullptr);
    glfwMakeContextCurrent(_window);
    
    gladLoadGL();
    _framebuffer = Framebuffer::create();
    _shader = Shader::create(util::read_text_file("data/shaders/ggx.vert"), util::read_text_file("data/shaders/ggx.frag"));
    _ground = Geometry::create("data/meshes/primitives/cube.obj",
                               glm::scale(glm::mat4{1.0f}, glm::vec3{20.0f, 20.0f, 20.0f}) * glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, -1.0f, 0.0f}));
}

void FrameRender::_render(const GameState &game_state, float time, glm::mat4 view_matrix, glm::mat4 projection_matrix) {
    
    glm::vec3 camera_position{glm::inverse(view_matrix) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    
    std::cout << game_state.playerHealth << std::endl;
    
    _shader->with([&](Shader &shader) {
        shader["view_matrix"] = view_matrix;
        shader["projection_matrix"] = projection_matrix;
        shader["camera_position"] = camera_position;
        shader["light_direction"] = glm::vec3{1.0f, 1.0f, 1.0f};
        shader["light_emission"] = glm::vec3{1.0f, 1.0f, 1.0f};
        _ground->draw(shader);
        for (auto &&organ : game_state.organs) {
            if (organ.organ_type != organ_type_t::PLAYER_HEAD) {
                organ.geometry->draw(shader);
            }
        }
        for (auto &&enemy : game_state.enemies) {
            enemy->geometry->draw(shader);
        }
        for (auto &&bullet : game_state.bullets) {
            bullet->geometry->draw(shader);
        }
    });
}
