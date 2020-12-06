#pragma once

#include <string>
#include <filesystem>
#include <functional>

#include "shader.h"
#include "rendereable.h"
#include "texture.h"
#include "framebuffer.h"
#include "mesh.h"
#include "camera.h"

const std::string DEF_CUBE_MAP_DIR_PATH = "../data/day_sky_env/";

class CubeMap {
protected:
    enum CubeMapFace {
        FRONT,
        BACK,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM
    };

    MeshEntity cube_entity_;

    // decode the string designation into a CubeMapFace
    static CubeMapFace decode_face(const std::string& kind);
    static CubeMapFace parse_path_name(const std::string& path_name);

public:
    CubeMap() : cube_entity_(MeshFactory::get().get_mesh_entity(DefMeshList::CUBE)) {}

    virtual void init(const std::string& dir_path, bool flip=false) = 0;
};

class GL_CubeMapEntity : public GL_CubeMapTex, public CubeMap {    
    // decode the face into the gl equivelant
    static uint32_t gl_decode_face(CubeMapFace face);
    // parse and decode the full path name into the gl equivelant
    uint32_t gl_decode_face(const std::string& path_name);
public:
    GL_CubeMapEntity(const std::string& dir_path) {
        init(dir_path);
    }
    GL_CubeMapEntity() : GL_CubeMapEntity(DEF_CUBE_MAP_DIR_PATH) {}
    
    void init(const std::string& dir_path, bool flip=false) override;
    void draw(ShaderProgram& programs);
};

class Environment {

    float fov_ = 50.0;

    int width_;
    int height_;

public:
    GL_CubeMapEntity cube_map_;
    GL_CubeMap_FBO fbo;
    GLCamera camera;

    Environment(std::shared_ptr<Camera> new_cam, int width, int height) : cube_map_(), fbo(width_ / 2.f), width_(width), height_(height), camera(new_cam) { viewport(width, height); }
    Environment(std::shared_ptr<Camera> new_cam, int width, int height, float fov) : Environment(new_cam, width, height) { 
        fov_ = fov;
        viewport(width, height); 
    }

    void draw(ShaderProgramCtx& programs);
    // draw the scene to a the fbo
    void draw_dynamic(ShaderProgramCtx& programs, MeshEntity& mesh_entity, MeshEntityList& mesh_entities, std::function<void(MeshEntity&)> draw_f);

    void set_width(int width) {
        width_ = width;
    }
    void set_height(int height) {
        height_ = height;
    }
    void viewport(int width, int height) {
        width_ = width;
        height_ = height;
        glViewport(0, 0, width, height);
    }
};