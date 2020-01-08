//
// Created by Mike on 1/4/2020.
//

#pragma once

#include <vector>
#include <memory>
#include <filesystem>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "shader.h"

struct Material {
    
    glm::vec3 diffuse_color;
    uint32_t diffuse_texture_handle;
    glm::vec3 specular_color;
    uint32_t specular_texture_handle;
    float roughness;
    
    Material() noexcept = default;
    
    Material(glm::vec3 diffuse_color, uint32_t diffuse_texture_handle, glm::vec3 specular_color, uint32_t specular_texture_handle, float roughness) noexcept
        : diffuse_color{diffuse_color},
          diffuse_texture_handle{diffuse_texture_handle},
          specular_color{specular_color},
          specular_texture_handle{specular_texture_handle},
          roughness{roughness} {}
};

struct Mesh {
    uint32_t vao_handle;
    uint32_t position_vbo_handle;
    uint32_t normal_vbo_handle;
    uint32_t texture_coord_vbo_handle;
    Material material;
    size_t vertex_count;
};

class Geometry {

private:
    std::vector<Mesh> _meshes;
    std::vector<glm::vec3> _position_buffer;
    std::vector<uint32_t> _index_buffer;
    uint32_t _shadow_vao_handle;
    uint32_t _shadow_vbo_handle;
    uint32_t _shadow_ebo_handle;
    glm::mat4 _transform{1.0f};

public:
    [[nodiscard]] static std::unique_ptr<Geometry> create(const std::filesystem::path &path, glm::mat4 initial_transform = glm::mat4{1.0f}) noexcept;
    void draw(Shader &shader);
    void shadow(Shader &shader);
    [[nodiscard]] glm::mat4 transform() const noexcept { return _transform; }
    void set_transform(glm::mat4 transform) noexcept { _transform = transform; }
    [[nodiscard]] std::vector<glm::vec3> &position_buffer() noexcept { return _position_buffer; }
    [[nodiscard]] std::vector<uint32_t> &index_buffer() noexcept { return _index_buffer; }
};
