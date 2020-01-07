//
// Created by Mike on 1/4/2020.
//

#include <cmath>
#include <iostream>
#include <filesystem>
#include <unordered_map>

#include <glad/glad.h>
#include <tinyobjloader/tiny_obj_loader.h>
#include <stb_image/stb_image.h>
#include <util/util.h>
#include <numeric>
#include "geometry.h"

std::unique_ptr<Geometry> Geometry::create(const std::filesystem::path &path, glm::mat4 initial_transform) noexcept {
    
    auto geometry = std::make_unique<Geometry>();
    
    tinyobj::ObjReaderConfig loader_config;
    loader_config.triangulate = true;
    tinyobj::ObjReader loader;
    loader.ParseFromFile(std::filesystem::absolute(path).string(), loader_config);
    
    auto directory = path.parent_path();
    std::unordered_map<std::string, uint32_t> loaded_textures;
    
    auto load_texture = [&](auto &&file_name) {
        
        if (file_name.empty()) { return 0u; }
        
        auto file_path = std::filesystem::absolute(directory / file_name).string();
        if (auto iter = loaded_textures.find(file_path); iter != loaded_textures.end()) { return iter->second; }
        
        auto width = 0;
        auto height = 0;
        auto num_channels = 0;
        std::cout << "Loading texture: " << file_path << std::endl;
        auto data = util::guard(stbi_load(file_path.c_str(), &width, &height, &num_channels, 4), [](uint8_t *p) { stbi_image_free(p); });
        if (*data == nullptr) {
            std::cerr << "Failed to load texture: " << file_path << std::endl;
            abort();
        }
        auto texture_handle = 0u;
        glGenTextures(1, &texture_handle);
        glBindTexture(GL_TEXTURE_2D, texture_handle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, *data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        loaded_textures.emplace(std::move(file_path), texture_handle);
        return texture_handle;
    };
    
    std::vector<Material> materials;
    for (auto &&material : loader.GetMaterials()) {
        auto diffuse_color = glm::vec3{material.diffuse[0], material.diffuse[1], material.diffuse[2]};
        auto diffuse_texture_handle = load_texture(material.diffuse_texname);
        auto specular_color = glm::vec3{material.specular[0], material.specular[1], material.specular[2]};
        auto specular_texture_handle = load_texture(material.specular_texname);
        auto roughness = std::sqrt(2.0f / (material.shininess + 2.0f));
        materials.emplace_back(diffuse_color, diffuse_texture_handle, specular_color, specular_texture_handle, roughness);
    }
    
    
    // process positions
    auto vertex_count = loader.GetAttrib().vertices.size() / 3;
    geometry->_position_buffer.resize(vertex_count);
    for (auto i = 0ul; i < vertex_count; i++) {
        geometry->_position_buffer[i] = initial_transform * glm::vec4{reinterpret_cast<const glm::vec3 *>(loader.GetAttrib().vertices.data())[i], 1.0f};
    }
    
    std::vector<glm::vec3> normal_buffer(vertex_count);
    std::vector<glm::vec2> texture_coord_buffer(vertex_count);
    auto normal_matrix = glm::transpose(glm::inverse(glm::mat3{initial_transform}));
    std::vector<std::vector<uint32_t>> index_buffers(materials.size());
    for (auto &&shape : loader.GetShapes()) {
        std::cout << "Info: loading mesh '" << shape.name << "'..." << std::endl;
        for (auto face = 0ul; face < shape.mesh.num_face_vertices.size(); face++) {
            auto material_id = shape.mesh.material_ids[face];
            auto i0 = shape.mesh.indices[face * 3ul];
            auto i1 = shape.mesh.indices[face * 3ul + 1ul];
            auto i2 = shape.mesh.indices[face * 3ul + 2ul];
            normal_buffer[i0.vertex_index] = glm::normalize(normal_matrix * reinterpret_cast<const glm::vec3 *>(loader.GetAttrib().normals.data())[i0.normal_index]);
            normal_buffer[i1.vertex_index] = glm::normalize(normal_matrix * reinterpret_cast<const glm::vec3 *>(loader.GetAttrib().normals.data())[i1.normal_index]);
            normal_buffer[i2.vertex_index] = glm::normalize(normal_matrix * reinterpret_cast<const glm::vec3 *>(loader.GetAttrib().normals.data())[i2.normal_index]);
            if (i0.texcoord_index >= 0 && i1.texcoord_index >= 0 && i2.texcoord_index >= 0) {
                texture_coord_buffer[i0.vertex_index] = reinterpret_cast<const glm::vec2 *>(loader.GetAttrib().texcoords.data())[i0.texcoord_index];
                texture_coord_buffer[i1.vertex_index] = reinterpret_cast<const glm::vec2 *>(loader.GetAttrib().texcoords.data())[i1.texcoord_index];
                texture_coord_buffer[i2.vertex_index] = reinterpret_cast<const glm::vec2 *>(loader.GetAttrib().texcoords.data())[i2.texcoord_index];
            }
            index_buffers[material_id].emplace_back(i0.vertex_index);
            index_buffers[material_id].emplace_back(i1.vertex_index);
            index_buffers[material_id].emplace_back(i2.vertex_index);
            geometry->_index_buffer.emplace_back(i0.vertex_index);
            geometry->_index_buffer.emplace_back(i1.vertex_index);
            geometry->_index_buffer.emplace_back(i2.vertex_index);
        }
    }
    
    glGenVertexArrays(1, &geometry->_vao_handle);
    glBindVertexArray(geometry->_vao_handle);
    
    glGenBuffers(1, &geometry->_position_vbo_handle);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->_position_vbo_handle);
    glBufferData(GL_ARRAY_BUFFER, geometry->_position_buffer.size() * sizeof(glm::vec3), geometry->_position_buffer.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), nullptr);
    
    glGenBuffers(1, &geometry->_normal_vbo_handle);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->_normal_vbo_handle);
    glBufferData(GL_ARRAY_BUFFER, normal_buffer.size() * sizeof(glm::vec3), normal_buffer.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(glm::vec3), nullptr);
    
    glGenBuffers(1, &geometry->_texture_coord_vbo_handle);
    glBindBuffer(GL_ARRAY_BUFFER, geometry->_texture_coord_vbo_handle);
    glBufferData(GL_ARRAY_BUFFER, texture_coord_buffer.size() * sizeof(glm::vec2), texture_coord_buffer.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(glm::vec2), nullptr);
    
    geometry->_meshes.resize(materials.size());
    for (auto i = 0ul; i < geometry->_meshes.size(); i++) {
        auto &&index_buffer = index_buffers[i];
        auto &&mesh = geometry->_meshes[i];
        mesh.material = materials[i];
        mesh.vertex_count = index_buffer.size();
        glGenBuffers(1, &mesh.ebo_handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo_handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer.size() * sizeof(uint32_t), index_buffer.data(), GL_STATIC_DRAW);
    }
    
    return geometry;
}

void Geometry::draw(Shader &shader) {
    
    shader["transform"] = _transform;
    
    glBindVertexArray(_vao_handle);
    for (auto &&mesh : _meshes) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.material.diffuse_texture_handle);
        shader["diffuse_texture"] = 0;
        shader["has_diffuse_texture"] = mesh.material.diffuse_texture_handle;
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mesh.material.specular_texture_handle);
        shader["specular_texture"] = 1;
        shader["has_specular_texture"] = mesh.material.specular_texture_handle;
        shader["diffuse_color"] = mesh.material.diffuse_color;
        shader["specular_color"] = mesh.material.specular_color;
        shader["roughness"] = mesh.material.roughness;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo_handle);
        glDrawElements(GL_TRIANGLES, mesh.vertex_count, GL_UNSIGNED_INT, nullptr);
    }
}
