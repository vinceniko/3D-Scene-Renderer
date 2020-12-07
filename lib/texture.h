#pragma once

#include "renderer.h"

class GL_Texture : public RenderObject {
protected:
    uint32_t tex_id_;
    GLenum target_ = GL_TEXTURE_2D;

    int width_;
    int height_;
public:
    GL_Texture() { init(); }
    GL_Texture(int width, int height) : width_(width), height_(height) { init(); }
    GL_Texture(GLenum target, int width, int height) : width_(width), height_(height), target_(target) { init(); }
    GL_Texture(GLenum target) : target_(target) { init(); }
    ~GL_Texture() { free(); }
    void init() override {
        glGenTextures(1, &tex_id_);

#ifdef DEBUG
        check_gl_error();
#endif
    }
    void bind() override {
        glBindTexture(target_, tex_id_);
        glActiveTexture(GL_TEXTURE0);
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

    int get_width() const {
        return width_;
    }
    int get_height() const {
        return height_;
    }
};
