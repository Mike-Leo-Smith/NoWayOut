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
    glfwWindowHint(GLFW_RESIZABLE, false);

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
    
    while (!glfwWindowShouldClose(window)) {
        
        glfwPollEvents();
        
        auto frame_width = 0;
        auto frame_height = 0;
        glfwGetFramebufferSize(window, &frame_width, &frame_height);
        
        DisplayState display_state{};
        display_state.time = static_cast<float>(glfwGetTime());
        display_state.mode = DisplayMode::STEREO;
        display->draw(display_state, frame_width, frame_height);
        
        glfwSwapBuffers(window);
    }

}