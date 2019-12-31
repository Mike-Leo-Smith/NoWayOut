//
// Created by Mike Smith on 2019/12/23.
// Copyright (c) 2019 Mike Smith. All rights reserved.
//

#include <core/util/util.h>
#include "shader.h"

std::unique_ptr<Shader> Shader::create(const std::string &vs_source, const std::string &fs_source) {
    
    int32_t success;
    char info[1024];
    
    auto compile = [&](uint32_t type, const char *src) {
        auto id = glCreateShader(type);
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(id, 1024, nullptr, info);
            glDeleteShader(id);
            throw std::runtime_error{util::serialize("Failed to compile shader: ", info)};
        }
        return id;
    };
    
    auto vs_id = util::guard(compile(GL_VERTEX_SHADER, vs_source.c_str()), [](uint32_t &id) { glDeleteShader(id); });
    auto fs_id = util::guard(compile(GL_FRAGMENT_SHADER, fs_source.c_str()), [](uint32_t &id) { glDeleteShader(id); });
    
    auto id = glCreateProgram();
    glAttachShader(id, *vs_id);
    glAttachShader(id, *fs_id);
    glLinkProgram(id);
    glGetProgramiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 1024, nullptr, info);
        glDeleteProgram(id);
        throw std::runtime_error{util::serialize("Failed to link program: ", info)};
    }
    return std::unique_ptr<Shader>{new Shader{id}};
}

Shader::UniformProxy Shader::operator[](const std::string &name) const noexcept {
    return UniformProxy{glGetUniformLocation(_id, name.c_str())};
}

