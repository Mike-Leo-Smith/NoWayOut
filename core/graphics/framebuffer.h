//
// Created by Mike on 1/4/2020.
//

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <glad/glad.h>

#include <util/util.h>

class Framebuffer : util::Noncopyable {
private:
    uint32_t _fbo_handle;
    uint32_t _texture_handle;

public:
    Framebuffer(uint32_t fbo, uint32_t texture) : _fbo_handle{fbo}, _texture_handle{texture} {}
    [[nodiscard]] static std::unique_ptr<Framebuffer> create() noexcept;
    void copy_pixels(std::vector<uint8_t> &buffer) const noexcept;
    
    template<typename Func>
    void with(Func &&func) {
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo_handle);
        func(*this);
    }
};
