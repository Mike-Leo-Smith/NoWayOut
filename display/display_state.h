//
// Created by Mike Smith on 2019/12/31.
//

#pragma once

#include <string_view>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <rapidjson/document.h>

#include <util/util.h>

enum struct DisplayMode {
    STEREO,
    CENTER
};

struct DisplayState {
    
    float time;
    DisplayMode mode;
    glm::mat4 view_matrix[2];
    glm::mat4 projection_matrix[2];
    
    [[nodiscard]] glm::vec3 front() const noexcept {
        return glm::normalize(glm::transpose(glm::mat3{view_matrix[0]}) * glm::vec3{0.0f, 0.0f, -1.0f});
    }
    
    [[nodiscard]] static DisplayState from_json_string(std::string_view json_string) {
        
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
    
    [[nodiscard]] std::string to_json_string() const noexcept {
        
        auto json_array_string = [](auto matrix) noexcept {
            std::ostringstream ss;
            ss << "[ " << std::setprecision(10);
            auto m = glm::value_ptr(matrix);
            for (auto i = 0ul; i < 15ul; i++) { ss << m[i] << ", "; }
            ss << m[15] << " ]";
            return ss.str();
        };
    
        return mode == DisplayMode::STEREO ?
               util::serialize(
                   std::setprecision(10),
                   "{\n",
                   R"(  "time": )", time, ",\n",
                   R"(  "mode": ")", "stereo", "\",\n",
                   R"(  "view_matrix_left": )", json_array_string(view_matrix[1]), ",\n",
                   R"(  "view_matrix_right": )", json_array_string(view_matrix[0]), ",\n",
                   R"(  "projection_matrix_left": )", json_array_string(projection_matrix[1]), ",\n",
                   R"(  "projection_matrix_right": )", json_array_string(projection_matrix[0]), "\n",
                   "}") :
               util::serialize(
                   std::setprecision(10),
                   "{\n",
                   R"(  "time": )", time, ",\n",
                   R"(  "mode": ")", "center", "\",\n",
                   R"(  "view_matrix": )", json_array_string(view_matrix[0]), ",\n",
                   R"(  "projection_matrix": )", json_array_string(projection_matrix[0]), "\n",
                   "}");
    }
};