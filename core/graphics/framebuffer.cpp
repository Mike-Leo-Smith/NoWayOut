//
// Created by Mike on 1/4/2020.
//

#include <iostream>
#include <config/config.h>

#include "framebuffer.h"

std::unique_ptr<Framebuffer> Framebuffer::create() noexcept {
    
    auto fbo = 0u;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    auto texture = 0u;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, config::eye_frame_width * 2ul, config::eye_frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    auto target = static_cast<uint32_t>(GL_COLOR_ATTACHMENT0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, target, GL_TEXTURE_2D, texture, 0);
    glDrawBuffers(1, &target);
    
    auto rbo = 0u;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, config::eye_frame_width * 2ul, config::eye_frame_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "G-Buffer Framebuffer not complete!" << std::endl;
        abort();
    }
    
    return std::make_unique<Framebuffer>(fbo, texture);
}

void Framebuffer::copy_pixels(std::vector<uint8_t> &buffer) const noexcept {
    buffer.resize(config::eye_frame_width * config::eye_frame_height * 2ul * 3ul);
    glBindTexture(GL_TEXTURE_2D, _texture_handle);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, buffer.data());
}
