#pragma once

#include <exception>

#include "renderer.h"
#include "cubemap.h"

class GL_FBO_Interface {
protected:
    uint32_t fbo_ = 0;
    virtual void init() {
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    virtual void unbind_() {};

public:
    GL_FBO_Interface() { init(); }
    GL_FBO_Interface(int fbo) : fbo_(fbo) {}
    ~GL_FBO_Interface() {
        glDeleteFramebuffers(1, &fbo_);

#ifdef DEBUG
        check_gl_error();
#endif
    }

    uint32_t get_fbo() const {
        return fbo_;
    }

    virtual void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    virtual void unbind(GL_FBO_Interface& main_fbo) {
        main_fbo.bind();
        unbind_();

#ifdef DEBUG
        check_gl_error();
#endif
    }
};

class GL_FBO_RBO_Interface : public GL_FBO_Interface {
protected:
    uint32_t rbo_ = 0;
    virtual void init() override {
        glGenRenderbuffers(1, &rbo_);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);

#ifdef DEBUG
        check_gl_error();
#endif
    }

public:
    GL_FBO_RBO_Interface() { init(); }
    GL_FBO_RBO_Interface(int fbo,int rbo) : GL_FBO_Interface(fbo), rbo_(rbo) {}
    ~GL_FBO_RBO_Interface() { 
        glDeleteRenderbuffers(1, &rbo_);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    uint32_t get_rbo() const {
        return rbo_;
    }

    virtual void bind() override {
        GL_FBO_Interface::bind();
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);

#ifdef DEBUG
        check_gl_error();
#endif
    }
};

template <typename Tex>
class GL_FBO_Tex_Interface : public GL_FBO_Interface {
protected:
    Tex tex_;

    virtual void unbind_() override {
        tex_.bind();
    }

public:
    GL_FBO_Tex_Interface(int width) : tex_(width) {}
    GL_FBO_Tex_Interface(int width, int height) : tex_(width, height) {}
    
    virtual void bind() override {
        GL_FBO_Interface::bind();
        tex_.bind();
        tex_.reset_viewport();

#ifdef DEBUG
        check_gl_error();
#endif
    }

    Tex& get_tex() {
        return tex_;
    }
};

template <typename Tex>
class GL_FBO_RBO_Tex_Interface : public GL_FBO_RBO_Interface {
protected:
    Tex tex_;
    virtual void unbind_() override {
        tex_.bind();
    }

public:
    GL_FBO_RBO_Tex_Interface(int width) : tex_(width) {}
    GL_FBO_RBO_Tex_Interface(int width, int height) : tex_(width, height) {}

    virtual void bind() override {
        GL_FBO_RBO_Interface::bind();
        tex_.bind();
        tex_.reset_viewport();

#ifdef DEBUG
        check_gl_error();
#endif
    }
    
    Tex& get_tex() {
        return tex_;
    }
};

class GL_CubeMap_FBO : public GL_FBO_RBO_Tex_Interface<GL_CubeMapTex> {
    void init() override {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tex_.get_width(), tex_.get_width());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

        tex_.init();
        
        // attach it to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, tex_.get_id(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("framebuffer incomplete");
        }

#ifdef DEBUG
        check_gl_error();
#endif
    }

public:
    GL_CubeMap_FBO(int width) : GL_FBO_RBO_Tex_Interface(width) { init(); }
    GL_CubeMap_FBO(int width, int height) : GL_FBO_RBO_Tex_Interface(width, height) { init(); }

    void bind() override {
        GL_FBO_RBO_Tex_Interface::bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    void next_face(size_t i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex_.get_id(), 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef DEBUG
        check_gl_error();
#endif
    }
};

class GL_Offscreen_FBO : public GL_FBO_RBO_Tex_Interface<GL_Texture> {
    void init() override {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_.get_width(), tex_.get_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tex_.get_width(), tex_.get_height());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

        // attach it to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_.get_id(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("framebuffer incomplete");
        }

#ifdef DEBUG
        check_gl_error();
#endif
    }

public:
    GL_Offscreen_FBO(int width) : GL_FBO_RBO_Tex_Interface(width) { init(); }
    GL_Offscreen_FBO(int width, int height) : GL_FBO_RBO_Tex_Interface(width, height) { init(); }
    void bind() override {
        GL_FBO_RBO_Tex_Interface::bind();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

#ifdef DEBUG
        check_gl_error();
#endif
    }
};

class GL_Depth_FBO : public GL_FBO_Tex_Interface<GL_Texture> {
    void init() override {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, tex_.get_width(), tex_.get_height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_.get_id(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("framebuffer incomplete");
        }

#ifdef DEBUG
        check_gl_error();
#endif
    }

public:
    GL_Depth_FBO(int width, int height) : GL_FBO_Tex_Interface(width, height) { init(); }

    void bind() override {
        GL_FBO_Tex_Interface::bind();

        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

#ifdef DEBUG
        check_gl_error();
#endif
    }
};
