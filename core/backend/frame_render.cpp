//
// Created by Mike Smith on 2019/12/31.
//

#include <rapidjson/document.h>
#include "frame_render.h"

DisplayState DisplayState::from_json_string(std::string_view json_string) {
    
    DisplayState state{};
    rapidjson::Document json;
    json.Parse(json_string.data(), json_string.size());
    
    using namespace std::string_view_literals;
    
    auto json_array_to_matrix = [](const auto &m) {
        return glm::mat4{
            m[0].GetFloat(), m[1].GetFloat(), m[2].GetFloat(), m[3].GetFloat(),
            m[4].GetFloat(), m[5].GetFloat(), m[6].GetFloat(), m[7].GetFloat(),
            m[8].GetFloat(), m[9].GetFloat(), m[10].GetFloat(), m[11].GetFloat(),
            m[12].GetFloat(), m[13].GetFloat(), m[14].GetFloat(), m[15].GetFloat()};
    };
    
    state.time = json["time"].GetFloat();
    if (json["mode"].GetString() == "stereo"sv) {
        state.mode = DisplayMode::STEREO;
        state.view_matrix[0] = json_array_to_matrix(json["view_matrix_left"].GetArray());
        state.view_matrix[1] = json_array_to_matrix(json["view_matrix_right"].GetArray());
        state.projection_matrix[0] = json_array_to_matrix(json["projection_matrix_left"].GetArray());
        state.projection_matrix[1] = json_array_to_matrix(json["projection_matrix_right"].GetArray());
    } else {
        state.mode = DisplayMode::CENTER;
        state.view_matrix[0] = json_array_to_matrix(json["view_matrix"].GetArray());
        state.projection_matrix[0] = json_array_to_matrix(json["projection_matrix"].GetArray());
    }
    
    return state;
}

void FrameRender::update(const GameState &game_state, const DisplayState &display_state) {
    
    if (_start_time == 0.0f) { _start_time = display_state.time; }
    
    auto frame_index = (display_state.time - _start_time) * 30.0f;
    auto frame_index_lower = static_cast<uint32_t>(std::floor(frame_index));
    auto frame_index_upper = static_cast<uint32_t>(std::ceil(frame_index));
    if (frame_index_upper >= _video_frames.size()) { return; }
    
    auto t = frame_index - frame_index_lower;
    auto raw_frame = _video_frames[frame_index_lower] * (1 - t) + _video_frames[frame_index_upper] * t;
	constexpr auto w = static_cast<int32_t>(config::eye_frame_width);
	constexpr auto h = static_cast<int32_t>(config::eye_frame_height);
    if (display_state.mode == DisplayMode::STEREO) {
        cv::resize(raw_frame, _eye_frame, cv::Size2i{w, h});
        _eye_frame.copyTo(_display_frame(cv::Rect{0, 0, w, h}));
        _eye_frame.copyTo(_display_frame(cv::Rect{w, 0, w, h}));
    } else {
        cv::resize(raw_frame, _display_frame, cv::Size2i{w * 2, h});
    }
    std::memmove(_frame_buffer.data(), _display_frame.data, _frame_buffer.size());
}

FrameRender::FrameRender()
    : _frame_buffer(config::eye_frame_width * 2ul * config::eye_frame_height * 3ul) {
    
    for (cv::VideoCapture camera{"data/test.mp4"}; camera.grab();) {
        _video_frames.emplace_back();
        camera.retrieve(_video_frames.back());
        std::cout << "Loaded frame #" << _video_frames.size() << std::endl;
        if (_video_frames.size() >= 500ul) {
            break;
        }
    }
    _display_frame.create(config::eye_frame_height, config::eye_frame_width * 2ul, CV_8UC3);
}
