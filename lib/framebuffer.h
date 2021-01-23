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

class GL_FBO : public GL_FBO_Interface, public Canvas {
public:
    GL_FBO(int fbo, int rbo, int width, int height) : GL_FBO_Interface(fbo), Canvas(width, height) {}
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

class GL_FBO_RBO : public GL_FBO_RBO_Interface, public Canvas {
public:
    GL_FBO_RBO(int fbo, int rbo, int width, int height) : GL_FBO_RBO_Interface(fbo, rbo), Canvas(width, height) {}
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
    GL_FBO_Tex_Interface(GLenum target, int width, int height) : tex_(target, width, height) {}
    
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

    void reset_viewport() {
        tex_.reset_viewport();
    }

    int get_width() {
        return get_tex().get_width();
    }
    int get_height() {
        return get_tex().get_height();
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
    GL_FBO_RBO_Tex_Interface(GLenum target, int width, int height) : tex_(target, width, height) {}

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
    
    void reset_viewport() {
        tex_.reset_viewport();
    }

    int get_width() {
        return get_tex().get_width();
    }
    int get_height() {
        return get_tex().get_height();
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
    GL_Depth_FBO(int width, int height, bool) : GL_FBO_Tex_Interface(width, height) { init(); }

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

class GL_Offscreen_FBO : public GL_FBO_Tex_Interface<GL_Texture> {
    GL_Texture depth_tex_;

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

        // GL_FBO_RBO_Interface::bind();
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
    GL_Offscreen_FBO(int width, int height) : GL_FBO_Tex_Interface(width, height), depth_tex_(width, height) { init(); }
    void bind() override {
        GL_FBO_Tex_Interface::bind();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    void bind(ShaderProgram& program) {
        Uniform("u_offscreen_tex").buffer(program, 0);
        Uniform("u_depth_map").buffer(program, 1);

        get_tex().bind(GL_TEXTURE0);
        depth_tex_.bind(GL_TEXTURE1);

#ifdef DEBUG
        check_gl_error();
#endif
    }

    template <typename FBO>
    void blit(FBO& main_fbo) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, get_fbo());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, main_fbo.get_fbo());
        glBlitFramebuffer(0, 0, get_tex().get_width(), get_tex().get_height(), 0, 0, main_fbo.get_width(), main_fbo.get_height(), GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    #ifdef DEBUG
            check_gl_error();
    #endif
    }

    void resize(int width, int height) {
        tex_.set_width(width);
        tex_.set_height(height);
        depth_tex_.set_width(width);
        depth_tex_.set_height(height);
        
        tex_.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_.get_width(), tex_.get_height(), 0, GL_RGBA, GL_FLOAT, NULL);
        depth_tex_.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, depth_tex_.get_width(), depth_tex_.get_height(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    }
};

class GL_Offscreen_FBO_Multisample : public GL_FBO_RBO_Tex_Interface<GL_Texture> {
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

        GL_FBO_RBO_Interface::bind();
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
    GL_Offscreen_FBO_Multisample(int width, int height) : GL_FBO_RBO_Tex_Interface(GL_TEXTURE_2D_MULTISAMPLE, width, height) { init(); }

    void bind() override {
        GL_FBO_RBO_Tex_Interface::bind();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    void bind(ShaderProgramCtx& programs) {
        programs.bind(ShaderPrograms::OFFSCREEN);
        Uniform("u_offscreen_tex").buffer(programs.get_selected_program(), 0);
        Uniform("u_depth_map").buffer(programs.get_selected_program(), 1);

        get_tex().bind(GL_TEXTURE0);

#ifdef DEBUG
        check_gl_error();
#endif
    }

    template <typename FBO>
    void blit(FBO& main_fbo) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, get_fbo());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, main_fbo.get_fbo());
        glBlitFramebuffer(0, 0, get_tex().get_width(), get_tex().get_height(), 0, 0, main_fbo.get_width(), main_fbo.get_height(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    #ifdef DEBUG
            check_gl_error();
    #endif
    }

    void resize(int width, int height) {
        tex_.set_width(width);
        tex_.set_height(height);
        
        tex_.bind();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_.get_width(), tex_.get_height(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        
        GL_FBO_RBO_Tex_Interface::bind();
        GL_FBO_RBO_Tex_Interface::reset_viewport();
    }
};
