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
