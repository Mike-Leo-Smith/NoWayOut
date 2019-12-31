//
// Created by Mike Smith on 2019/10/8.
// Copyright (c) 2019 Mike Smith. All rights reserved.
//

#pragma once

#include <memory>
#include <glad/glad.h>

class Shader {

public:
    struct UniformProxy {
        
        int32_t location;
        
        UniformProxy &operator=(bool value) noexcept {
            glUniform1i(location, value);
            return *this;
        }
        
        UniformProxy &operator=(int32_t value) noexcept {
            glUniform1i(location, value);
            return *this;
        }
        
        UniformProxy &operator=(uint32_t value) noexcept {
            glUniform1i(location, value);
            return *this;
        }
        
        UniformProxy &operator=(float value) noexcept {
            glUniform1f(location, value);
            return *this;
        }
    };

private:
    uint32_t _id;
    explicit Shader(uint32_t id) noexcept : _id{id} {};

public:
    Shader(Shader &&) = delete;
    Shader(const Shader &) = delete;
    Shader &operator=(Shader &&) = delete;
    Shader &operator=(const Shader &) = delete;
    
    ~Shader() noexcept { glDeleteProgram(_id); }
    
    [[nodiscard]] static std::unique_ptr<Shader> create(const std::string &vs_source, const std::string &fs_source);
    [[nodiscard]] UniformProxy operator[](const std::string &name) const noexcept;
    
    template<typename Func>
    void with(Func &&func) noexcept {
        glUseProgram(_id);
        func(*this);
        glUseProgram(0);
    }
};
