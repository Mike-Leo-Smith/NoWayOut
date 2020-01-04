//
// Created by Mike on 1/4/2020.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Material {
    glm::vec3 base_color;
    uint32_t base_color_texture_handle;
    glm::vec3 specular_color;
    uint32_t specular_color_texture_handle;
    float roughness;
};

struct MeshNode {
    uint32_t _vao_handle;
    uint32_t _vbo_handle;
    glm::mat4 _transform;
    uint32_t _material_index;
};

class Mesh {

private:
    std::vector<Material> _materials;
    

};
