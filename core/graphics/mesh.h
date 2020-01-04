//
// Created by Mike on 1/4/2020.
//

#pragma once

#include <glm/glm.hpp>

struct Material {
    glm::vec3 base_color;
    uint32_t base_color_texture_handle;
    glm::vec3 specular_color;
    uint32_t specular_color_texture_handle;
};

class Mesh {

};
