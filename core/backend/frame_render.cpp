//
// Created by Mike Smith on 2019/12/31.
//

#include "frame_render.h"
#include "game_logic.h"

void FrameRender::update(const GameState &game_state, const DisplayState &display_state) {
    
    _shadow_pass(game_state);
    
    _framebuffer->with([&](auto &framebuffer) {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glViewport(0, 0, config::eye_frame_width * 2ul, config::eye_frame_height);
        glScissor(0, 0, config::eye_frame_width * 2ul, config::eye_frame_height);
        glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
        glClear(static_cast<uint32_t>(GL_COLOR_BUFFER_BIT) | static_cast<uint32_t>(GL_DEPTH_BUFFER_BIT));
        glm::mat4 head_transform{1.0f};
        for (auto &&organ : game_state.organs) {
            if (organ.organ_type == organ_type_t::PLAYER_HEAD) {
                auto origin = organ.obj->getWorldTransform().getOrigin();
                head_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{-origin.x(), -origin.y(), -origin.z()});
                break;
            }
        }
        if (display_state.mode == DisplayMode::CENTER) {
            _render(game_state, display_state.view_matrix[0] * head_transform, display_state.projection_matrix[0]);
        } else {
            glViewport(0, 0, config::eye_frame_width, config::eye_frame_height);
            glScissor(0, 0, config::eye_frame_width, config::eye_frame_height);
            _render(game_state, display_state.view_matrix[0] * head_transform, display_state.projection_matrix[0]);
            glViewport(config::eye_frame_width, 0, config::eye_frame_width, config::eye_frame_height);
            glScissor(config::eye_frame_width, 0, config::eye_frame_width, config::eye_frame_height);
            _render(game_state, display_state.view_matrix[1] * head_transform, display_state.projection_matrix[1]);
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
    
    _context = glfwCreateWindow(config::eye_frame_width * 2ul, config::eye_frame_height, "Backend", nullptr, nullptr);
    glfwHideWindow(_context);
    glfwMakeContextCurrent(_context);
    
    gladLoadGL();
    _framebuffer = Framebuffer::create();
    _shader = Shader::create(util::read_text_file("data/shaders/ggx.vert"), util::read_text_file("data/shaders/ggx.frag"));
    _shadow_shader = Shader::create(util::read_text_file("data/shaders/shadow.vert"), util::read_text_file("data/shaders/shadow.frag"));
    _ground = Geometry::create("data/meshes/primitives/plane.obj",
                               glm::scale(glm::mat4{1.0f}, glm::vec3{50.0f, 50.0f, 50.0f}));
    
    _create_shadow_map();
}

void FrameRender::_render(const GameState &game_state, glm::mat4 view_matrix, glm::mat4 projection_matrix) {
    
    glm::vec3 camera_position{glm::inverse(view_matrix) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    
    _shader->with([&](Shader &shader) {
        shader["view_matrix"] = view_matrix;
        shader["projection_matrix"] = projection_matrix;
        shader["camera_position"] = camera_position;
        shader["light_direction"] = _light_direction;
        shader["light_emission"] = _light_emission;
        shader["light_transform"] = _light_transform;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _shadow_texture_handle);
        shader["shadow_map"] = 0;
        _ground->draw(shader);
        for (auto &&organ : game_state.organs) {
            if (organ.organ_type != organ_type_t::PLAYER_HEAD) {
                organ.geometry->draw(shader, organ.transform());
            }
        }
        for (auto &&enemy : game_state.enemies) {
            enemy->geometry->draw(shader, enemy->transform());
        }
        for (auto &&bullet : game_state.bullets) {
            bullet->geometry->draw(shader, bullet->transform());
        }
    });
}

void FrameRender::_create_shadow_map() {
    
    glGenTextures(1, &_shadow_texture_handle);
    glBindTexture(GL_TEXTURE_2D, _shadow_texture_handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glm::vec4 border_color{1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &border_color.x);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 2048u, 2048u, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    glGenFramebuffers(1, &_shadow_fbo_handle);
    glBindFramebuffer(GL_FRAMEBUFFER, _shadow_fbo_handle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadow_texture_handle, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error{util::serialize("Shadowmap framebuffer incomplete!")};
    }
    
}

void FrameRender::_shadow_pass(const GameState &game_state) {
    
    glBindFramebuffer(GL_FRAMEBUFFER, _shadow_fbo_handle);
    glViewport(0, 0, 2048, 2048);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    auto light_projection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, 1.0f, 25.0f);
    auto light_view = glm::lookAt(10.0f * _light_direction, glm::vec3{}, glm::vec3{0.0f, 1.0f, 0.0f});
    _light_transform = light_projection * light_view;
    
    _shadow_shader->with([&](auto &shader) {
        shader["light_transform"] = _light_transform;
        _ground->shadow(shader, glm::mat4());
        for (auto &&organ : game_state.organs) {
            if (organ.organ_type != organ_type_t::PLAYER_HEAD) {
                organ.geometry->shadow(shader, organ.transform());
            }
        }
        for (auto &&enemy : game_state.enemies) {
            enemy->geometry->shadow(shader, enemy->transform());
        }
        for (auto &&bullet : game_state.bullets) {
            bullet->geometry->shadow(shader, bullet->transform());
        }
    });
    glDisable(GL_CULL_FACE);
}
