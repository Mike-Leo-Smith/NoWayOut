//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <vector>
#include <string_view>
#include <glm/glm.hpp>

#include <opencv2/opencv.hpp>

#include "config.h"

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
    std::vector<cv::Mat> _video_frames;
    cv::Mat _eye_frame;
    cv::Mat _display_frame;
    float _start_time{0.0f};

public:
    FrameRender();
    [[nodiscard]] const std::vector<uint8_t> &frame() const noexcept { return _frame_buffer; }
    void update(const GameState &game_state, const DisplayState &display_state);
    
    template<typename ...Args>
    [[nodiscard]] static std::unique_ptr<FrameRender> create(Args &&...args) noexcept {
        return std::make_unique<FrameRender>(std::forward<Args>(args)...);
    }
};