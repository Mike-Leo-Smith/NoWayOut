//
// Created by Mike on 1/4/2020.
//

#pragma once

#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "shader.h"

struct Material {
    
    glm::vec3 diffuse_color;
    uint32_t diffuse_texture_handle;
    glm::vec3 specular_color;
    uint32_t specular_texture_handle;
    float roughness;
    
    Material() noexcept = default;
    
    Material(glm::vec3 diffuse_color, uint32_t diffuse_texture_handle,
             glm::vec3 specular_color, uint32_t specular_texture_handle,
             float roughness) noexcept
        : diffuse_color{diffuse_color},
          diffuse_texture_handle{diffuse_texture_handle},
          specular_color{specular_color},
          specular_texture_handle{specular_texture_handle}, roughness{roughness} {
    }
};

struct Mesh {
    Material material;
    uint32_t ebo_handle;
    size_t vertex_count;
};

class Geometry {

private:
    uint32_t _vao_handle;
    uint32_t _position_vbo_handle;
    uint32_t _normal_vbo_handle;
    uint32_t _texture_coord_vbo_handle;
    std::vector<glm::vec3> _position_buffer;
    std::vector<int32_t> _index_buffer;
    std::vector<Mesh> _meshes;
    glm::mat4 _transform{1.0f};

public:
    [[nodiscard]] static std::unique_ptr<Geometry>
    create(const std::filesystem::path &path,
           glm::mat4 initial_transform = glm::mat4{1.0f}) noexcept;
    void draw(Shader &shader);
    void set_transform(glm::mat4 transform) noexcept { _transform = transform; }
};
