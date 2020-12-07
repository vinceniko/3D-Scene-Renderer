#pragma once

#include <exception>

#include "renderer.h"
#include "cubemap.h"

class GL_CubeMap_FBO {
    uint32_t fbo_ = 0;
    uint32_t rbo_ = 0;

public:
    GL_CubeMapTex tex_;
    GL_CubeMap_FBO(int width) : tex_(width) { init(); }
    ~GL_CubeMap_FBO() { free(); }

    void init() {
        glGenFramebuffers(1, &fbo_);
        glGenRenderbuffers(1, &rbo_);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tex_.get_width(), tex_.get_width());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo_);

        tex_.init();

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
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
        tex_.bind();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#ifdef DEBUG
        check_gl_error();
#endif
    }
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        tex_.bind();
    }
    const uint32_t& get_id() const {
        return fbo_;
    }
    const GL_CubeMapTex& get_tex() const {
        return tex_;
    }
    void next_face(size_t i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex_.get_id(), 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glClear(GL_DEPTH_BUFFER_BIT);
    }
};
