#pragma once

#include "helpers.h"

#include <string>
#include <filesystem>

#include "shader.h"
#include "mesh.h"
#include "camera.h"

const std::string DEF_CUBE_MAP_DIR_PATH = "../data/night_env/";

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
    virtual void load(const std::string& dir_path) = 0;
};

class GLCubeMap : public CubeMap {
    std::reference_wrapper<ShaderProgramCtx> programs_;
    
    MeshEntity cube_entity_;

    uint32_t tex_id_;

    // decode the face into the gl equivelant
    static uint32_t gl_decode_face(CubeMapFace face);
    // parse and decode the full path name into the gl equivelant
    uint32_t gl_decode_face(const std::string& path_name);
public:
    GLCubeMap(ShaderProgramCtx& programs, MeshFactory& mesh_factory, const std::string& dir_path) : programs_(programs), cube_entity_(mesh_factory.get_mesh_entity(DefMeshList::CUBE)) {
        cube_entity_.scale(glm::mat4(1.f), Spatial::ScaleDir::Out, 2.0);
        load(dir_path);
    }
    GLCubeMap(ShaderProgramCtx& programs, MeshFactory& mesh_factory) : GLCubeMap(programs, mesh_factory, DEF_CUBE_MAP_DIR_PATH) {}

    void load(const std::string& dir_path) override;

    void draw();
};

class Environment {
    ShaderProgramCtx& programs_;
    GLCubeMap cube_map_;

    float fov_ = 90.0;

public:
    GLCamera camera;

    Environment(ShaderProgramCtx& programs, MeshFactory& mesh_factory, std::shared_ptr<Camera> new_cam) : programs_(programs), cube_map_(programs, mesh_factory), camera(programs, new_cam) {}
    Environment(ShaderProgramCtx& programs, MeshFactory& mesh_factory, std::shared_ptr<Camera> new_cam, float fov) : programs_(programs), cube_map_(programs, mesh_factory), camera(programs, new_cam), fov_(fov) {}

    void draw();
};