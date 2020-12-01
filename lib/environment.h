#pragma once

#include "helpers.h"

#include <string>
#include <filesystem>

#include "shader.h"
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

    // decode the string designation into a CubeMapFace
    static CubeMapFace decode_face(const std::string& kind);
    static CubeMapFace parse_path_name(const std::string& path_name);

public:
    virtual void load(const std::string& dir_path, bool flip) = 0;
};

class GLCubeMap : public CubeMap {    
    MeshEntity cube_entity_;

    uint32_t tex_id_;

    // decode the face into the gl equivelant
    static uint32_t gl_decode_face(CubeMapFace face);
    // parse and decode the full path name into the gl equivelant
    uint32_t gl_decode_face(const std::string& path_name);
public:
    GLCubeMap(MeshFactory& mesh_factory, const std::string& dir_path) : cube_entity_(mesh_factory.get_mesh_entity(DefMeshList::CUBE)) {
        // cube_entity_.scale(glm::mat4(1.f), Spatial::ScaleDir::Out, 2.0);
        load(dir_path);
    }
    GLCubeMap(MeshFactory& mesh_factory) : GLCubeMap(mesh_factory, DEF_CUBE_MAP_DIR_PATH) {}

    void load(const std::string& dir_path, bool flip=false) override;

    void draw(ShaderProgramCtx& programs);
};

class Environment {
    GLCubeMap cube_map_;

    float fov_ = 50.0;

public:
    GLCamera camera;

    Environment(MeshFactory& mesh_factory, std::shared_ptr<Camera> new_cam) : cube_map_(mesh_factory), camera(new_cam) {}
    Environment(MeshFactory& mesh_factory, std::shared_ptr<Camera> new_cam, float fov) : cube_map_(mesh_factory), camera(new_cam), fov_(fov) {}

    void draw(ShaderProgramCtx& programs);
};