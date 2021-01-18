#pragma once

#include "renderer.h"

class GL_Texture : public RenderObject, public Canvas {
protected:
    GLenum target_ = GL_TEXTURE_2D;

public:
    uint32_t tex_id_;
    GL_Texture() { init(); }
    GL_Texture(int width, int height) : Canvas(width, height) { init(); }
    GL_Texture(GLenum target, int width, int height) : Canvas(width, height), target_(target) { init(); }
    GL_Texture(GLenum target) : target_(target) { init(); }
    ~GL_Texture() { free(); }
    void init() override {
        glGenTextures(1, &tex_id_);
        bind();

#ifdef DEBUG
        check_gl_error();
#endif
    }
    void bind() override {
        bind(GL_TEXTURE0);
    }
    virtual void bind(uint32_t tex_unit) {
        glActiveTexture(tex_unit);
        glBindTexture(target_, tex_id_);
#ifdef DEBUG
        check_gl_error();
#endif
    }
    void free() override {
        glDeleteTextures(1, &tex_id_);
        tex_id_ = 0;
#ifdef DEBUG
        check_gl_error();
#endif
    }

    const uint32_t& get_id() const {
        return tex_id_;
    }
};
