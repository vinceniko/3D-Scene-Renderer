#pragma once

#include "renderer.h"

class GL_Texture : public RenderObject {
protected:
    uint32_t tex_id_;
    GLenum target_;

public:    
    GL_Texture(GLenum target) : target_(target) { init(); }
    ~GL_Texture() { free(); }
    void init() override {
        glGenTextures(1, &tex_id_);
    }
    void bind() override {
        glBindTexture(target_, tex_id_);
    }
    void free() override {
        glDeleteTextures(1, &tex_id_);
    }

    uint32_t& get_id() {
        return tex_id_;
    }
};

class GL_CubeMapTex : public GL_Texture {    
public:
    GL_CubeMapTex() : GL_Texture(GL_TEXTURE_CUBE_MAP) {}
};
