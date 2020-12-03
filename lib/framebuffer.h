#pragma once

#include <exception>

#include "renderer.h"
#include "texture.h"

class GL_CubeMap_FBO {
    uint32_t fbo_ = 0;

public:
    GL_CubeMapTex tex_;
    GL_CubeMap_FBO(int width) : tex_(width) { init(); }
    ~GL_CubeMap_FBO() { free(); }

    void init() {
        glGenFramebuffers(1, &fbo_);
        bind();

        // // generate texture
        tex_.bind();
        for (int i = 0; i < 6; i++) {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, tex_.get_width(), tex_.get_width(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
            );
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // attach it to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, tex_.get_id(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("framebuffer incomplete");
        }

        unbind();
    }
    void free() {
        glDeleteFramebuffers(1, &fbo_);
    }
    void bind() {
        tex_.bind();
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        // const float color[4] = { 1.0, 1.0, 1.0, 1.0 };
        // glClearBufferfv(GL_FRAMEBUFFER, fbo_, color);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    }
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        tex_.bind();
    }
    uint32_t& get_id() {
        return fbo_;
    }
};
