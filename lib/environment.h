#pragma once

#include <string>
#include <filesystem>

#include "shader.h"
#include "rendereable.h"
#include "texture.h"
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
    CubeMap(MeshFactory& mesh_factory) : cube_entity_(mesh_factory.get_mesh_entity(DefMeshList::CUBE)) {}

    virtual void init(const std::string& dir_path, bool flip) = 0;
};

class GL_CubeMapEntity : public GL_CubeMapTex, public CubeMap {    
    // decode the face into the gl equivelant
    static uint32_t gl_decode_face(CubeMapFace face);
    // parse and decode the full path name into the gl equivelant
    uint32_t gl_decode_face(const std::string& path_name);
public:
    GL_CubeMapEntity(MeshFactory& mesh_factory, const std::string& dir_path) : GL_CubeMapTex(), CubeMap(mesh_factory) {
        init(dir_path);
    }
    GL_CubeMapEntity(MeshFactory& mesh_factory) : GL_CubeMapEntity(mesh_factory, DEF_CUBE_MAP_DIR_PATH) {}
    
    void init(const std::string& dir_path, bool flip=false) override;
    void draw(ShaderProgram& programs);
};

class Environment {
    GL_CubeMapEntity cube_map_;

    float fov_ = 50.0;

public:
    GLCamera camera;

    Environment(MeshFactory& mesh_factory, std::shared_ptr<Camera> new_cam) : cube_map_(mesh_factory), camera(new_cam) {}
    Environment(MeshFactory& mesh_factory, std::shared_ptr<Camera> new_cam, float fov) : cube_map_(mesh_factory), camera(new_cam), fov_(fov) {}

    void bind();
    void draw(ShaderProgramCtx& programs);
};