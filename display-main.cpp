//
// Created by Mike Smith on 2019/12/31.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <config/config.h>
#include <display/display.h>

int main() {
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
#endif
    
    constexpr auto window_width = static_cast<int32_t>(config::eye_frame_width * 2);
    constexpr auto window_height = static_cast<int32_t>(config::eye_frame_height);
    auto window = glfwCreateWindow(window_width, window_height, "NoWayOut - Display", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    
    gladLoadGL();
    
    auto display = Display::create();
    display->initialize();
    
    bool is_stereo_display = true;
    
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 right{1.0f, 0.0f, 0.0f};
    
    while (!glfwWindowShouldClose(window)) {
        
        glfwPollEvents();
        
        if (glfwGetKey(window, GLFW_KEY_SPACE)) {
            is_stereo_display = !is_stereo_display;
        } else if (glfwGetKey(window, GLFW_KEY_W)) {
            up = glm::rotate(glm::mat4{1.0f}, -0.02f, right) * glm::vec4{up, 1.0f};
        } else if (glfwGetKey(window, GLFW_KEY_S)) {
            up = glm::rotate(glm::mat4{1.0f}, 0.02f, right) * glm::vec4{up, 1.0f};
        } else if (glfwGetKey(window, GLFW_KEY_A)) {
            right = glm::rotate(glm::mat4{1.0f}, 0.02f, up) * glm::vec4{right, 1.0f};
        } else if (glfwGetKey(window, GLFW_KEY_D)) {
            right = glm::rotate(glm::mat4{1.0f}, -0.02f, up) * glm::vec4{right, 1.0f};
        }
        
        auto frame_width = 0;
        auto frame_height = 0;
        glfwGetFramebufferSize(window, &frame_width, &frame_height);
        
        DisplayState display_state{};
        display_state.time = static_cast<float>(glfwGetTime());
        auto view_matrix = glm::lookAt(glm::vec3{}, glm::cross(up, right), up);
        if (is_stereo_display) {
            display_state.mode = DisplayMode::STEREO;
            display_state.view_matrix[0] = glm::translate(glm::mat4{1.0f}, glm::vec3{0.03f, 0.0f, 0.0f}) * view_matrix;
            display_state.view_matrix[1] = glm::translate(glm::mat4{1.0f}, glm::vec3{-0.03f, 0.0f, 0.0f}) * view_matrix;
            auto projection_matrix = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 25.0f);
            display_state.projection_matrix[0] = projection_matrix;
            display_state.projection_matrix[1] = projection_matrix;
        } else {
            display_state.mode = DisplayMode::CENTER;
            display_state.view_matrix[0] = view_matrix;
            display_state.projection_matrix[0] = glm::perspective(glm::radians(60.0f), 2.0f, 0.1f, 100.0f);
        }
        display->draw(display_state, frame_width, frame_height);
        
        glfwSwapBuffers(window);
    }
    
}
