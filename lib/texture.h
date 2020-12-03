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
    }
    void bind() override {
        glBindTexture(target_, tex_id_);
        glActiveTexture(GL_TEXTURE0);
    }
    void free() override {
        glDeleteTextures(1, &tex_id_);
    }

    uint32_t& get_id() {
        return tex_id_;
    }

    int get_width() {
        return width_;
    }
    int get_height() {
        return height_;
    }
};

class GL_CubeMapTex : public GL_Texture {    
public:    
    GL_CubeMapTex() : GL_Texture(GL_TEXTURE_CUBE_MAP) {}
    GL_CubeMapTex(int width) : GL_Texture(GL_TEXTURE_CUBE_MAP, width, width) {}
};
