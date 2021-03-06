#pragma once

#include <exception>

#include "renderer.h"
#include "cubemap.h"

class FBO : public Canvas, public RenderObj {
protected:
    uint32_t fbo_ = 0;
    virtual void init() {
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    // for virtual unbinding
    virtual void unbind_() {};

public:
    FBO() { init(); }
    FBO(int fbo) : fbo_(fbo) {}
    FBO(int fbo, int width, int height) : fbo_(fbo), Canvas(width, height) {}
    ~FBO() {
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
        reset_viewport();

#ifdef DEBUG
        check_gl_error();
#endif
    }
    virtual void unbind(FBO& main_fbo) {
        main_fbo.bind();
        unbind_();

#ifdef DEBUG
        check_gl_error();
#endif
    }

    void blit(FBO& main_fbo, int bits = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, int filter = GL_NEAREST) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, get_fbo());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, main_fbo.get_fbo());
        glBlitFramebuffer(0, 0, get_width(), get_height(), 0, 0, main_fbo.get_width(), main_fbo.get_height(), bits, filter);
        main_fbo.bind();
#ifdef DEBUG
        check_gl_error();
#endif
    }

    virtual void blit_from(FBO& main_fbo) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, main_fbo.get_fbo());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, get_fbo());
        glBlitFramebuffer(0, 0, main_fbo.get_width(), main_fbo.get_height(), 0, 0, get_width(), get_height(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
        main_fbo.bind();
#ifdef DEBUG
        check_gl_error();
#endif
    }
};

class FBO_RBO : public FBO {
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
    FBO_RBO() { init(); }
    FBO_RBO(int fbo, int rbo) : FBO(fbo), rbo_(rbo) {}
    FBO_RBO(int fbo, int rbo, int width, int height) : FBO(fbo, width, height), rbo_(rbo) {}

    ~FBO_RBO() {
        glDeleteRenderbuffers(1, &rbo_);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    uint32_t get_rbo() const {
        return rbo_;
    }

    virtual void bind() override {
        FBO::bind();
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_);

#ifdef DEBUG
        check_gl_error();
#endif
    }
};

template <typename Tex>
class FBO_Tex_Interface_Common {
protected:
    Tex tex_;

public:
    FBO_Tex_Interface_Common(int width) : tex_(width) {}
    FBO_Tex_Interface_Common(int width, int height) : tex_(width, height) {}
    FBO_Tex_Interface_Common(GLenum target, int width, int height) : tex_(target, width, height) {}

    void bind() {
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
class FBO_Tex_Interface : public FBO, public FBO_Tex_Interface_Common<Tex> {
protected:
    virtual void unbind_() override {
        FBO_Tex_Interface_Common<Tex>::tex_.bind();
    }
public:
    virtual void bind() override {
        FBO::bind();
        FBO_Tex_Interface_Common<Tex>::bind();
    }
    using FBO_Tex_Interface_Common<Tex>::FBO_Tex_Interface_Common;

    virtual int get_width() override {
        return FBO_Tex_Interface_Common<Tex>::get_tex().get_width();
    }
    virtual int get_height() override {
        return FBO_Tex_Interface_Common<Tex>::get_tex().get_height();
    }
    virtual void reset_viewport() override {
        FBO_Tex_Interface_Common<Tex>::tex_.reset_viewport();
    }
};

template <typename Tex>
class FBO_RBO_Tex_Interface : public FBO_RBO, public FBO_Tex_Interface_Common<Tex> {
protected:
    virtual void unbind_() override {
        FBO_Tex_Interface_Common<Tex>::tex_.bind();
    }
public:
    virtual void bind() override {
        FBO_RBO::bind();
        FBO_Tex_Interface_Common<Tex>::bind();
    }
    using FBO_Tex_Interface_Common<Tex>::FBO_Tex_Interface_Common;

    virtual int get_width() override {
        return FBO_Tex_Interface_Common<Tex>::get_tex().get_width();
    }
    virtual int get_height() override {
        return FBO_Tex_Interface_Common<Tex>::get_tex().get_height();
    }
    virtual void reset_viewport() override {
        FBO_Tex_Interface_Common<Tex>::tex_.reset_viewport();
    }
};

class CubeMap_FBO : public FBO_RBO_Tex_Interface<CubeMapTex> {
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
    CubeMap_FBO(int width) : FBO_RBO_Tex_Interface(width) { init(); }
    CubeMap_FBO(int width, int height) : FBO_RBO_Tex_Interface(width, height) { init(); }

    void bind() override {
        FBO_RBO_Tex_Interface::bind();

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

class Depth_FBO : public FBO_Tex_Interface<Texture> {
    void init() override {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, tex_.get_width(), tex_.get_height(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
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
    Depth_FBO(int width, int height) : FBO_Tex_Interface(width, height) { init(); }
    Depth_FBO(int width, int height, bool) : FBO_Tex_Interface(width, height) { init(); }

    void bind() override {
        FBO_Tex_Interface::bind();

        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

#ifdef DEBUG
        check_gl_error();
#endif
    }
};

class Offscreen_FBO : public FBO_Tex_Interface<Texture> {
    Texture depth_tex_;

    virtual void init() override {
        tex_.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_.get_width(), tex_.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach it to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_.get_id(), 0);

        depth_tex_.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, depth_tex_.get_width(), depth_tex_.get_height(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth_tex_.get_id(), 0);

        // FBO_RBO::bind();
        // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tex_.get_width(), tex_.get_height());
        // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("framebuffer incomplete");
        }

#ifdef DEBUG
        check_gl_error();
#endif
    }

public:
    Offscreen_FBO(int width, int height) : FBO_Tex_Interface(width, height), depth_tex_(width, height) { init(); }
    void bind() override {
        FBO_Tex_Interface::bind();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    void bind_offscreen() {
        renderer_->bind(ShaderPrograms::OFFSCREEN);
        Uniform("u_offscreen_tex").buffer(0);
        Uniform("u_depth_map").buffer(1);

        get_tex().bind(GL_TEXTURE0);
        depth_tex_.bind(GL_TEXTURE1);

#ifdef DEBUG
        check_gl_error();
#endif
    }

    void resize(int width, int height) override {
        tex_.set_width(width);
        tex_.set_height(height);
        depth_tex_.set_width(width);
        depth_tex_.set_height(height);

        tex_.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_.get_width(), tex_.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        depth_tex_.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, depth_tex_.get_width(), depth_tex_.get_height(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    }
};

class Offscreen_FBO_Multisample : public FBO_RBO_Tex_Interface<Texture> {
    void init() override {
        tex_.bind();
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, tex_.get_width(), tex_.get_height(), GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef DEBUG
        check_gl_error();
#endif
        // attach it to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex_.get_id(), 0);
#ifdef DEBUG
        check_gl_error();
#endif

        FBO_RBO::bind();
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, tex_.get_width(), tex_.get_height());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("framebuffer incomplete");
        }

#ifdef DEBUG
        check_gl_error();
#endif
    }

public:
    Offscreen_FBO_Multisample(int width, int height) : FBO_RBO_Tex_Interface(GL_TEXTURE_2D_MULTISAMPLE, width, height) { init(); }

    void bind() override {
        FBO_RBO_Tex_Interface::bind();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    void bind_offscreen() {
        renderer_->bind(ShaderPrograms::OFFSCREEN);
        Uniform("u_offscreen_tex").buffer(0);
        Uniform("u_depth_map").buffer(1);

        get_tex().bind(GL_TEXTURE0);

#ifdef DEBUG
        check_gl_error();
#endif
    }

    void resize(int width, int height) override {
        tex_.set_width(width);
        tex_.set_height(height);

        tex_.bind();
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, tex_.get_width(), tex_.get_height(), GL_TRUE);
        
        FBO_RBO::bind();
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, tex_.get_width(), tex_.get_height());
    }
};
