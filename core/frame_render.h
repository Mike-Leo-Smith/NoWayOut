//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <vector>
#include <string_view>
#include <glm/glm.hpp>

enum struct DisplayMode {
    STEREO,
    CENTER
};

struct DisplayState {
    
    float time;
    DisplayMode mode;
    glm::mat4 view_matrix[2];
    glm::mat4 projection_matrix[2];
    
    [[nodiscard]] static DisplayState from_json_string(std::string_view json_string);
};

struct GameState;

class FrameRender {

private:
    std::vector<uint8_t> _frame_buffer;

public:
    void initialize(const GameState &game_state);
    
};
