#pragma once

#include <string>
#include <filesystem>
#include <functional>

#include "renderer.h"
#include "texture.h"
#include "mesh.h"
#include "camera.h"

const std::string DEF_CUBE_MAP_DIR_PATH = "../data/day_sky_env/";

// not initialized when constructed
// init should be explicitly called
class GL_CubeMapTex : public GL_Texture {
public:
    GL_CubeMapTex() : GL_Texture(GL_TEXTURE_CUBE_MAP) {}
    GL_CubeMapTex(int width) : GL_Texture(GL_TEXTURE_CUBE_MAP, width, width) {}

    // create an empty cubemap
    void init() override;
    void bind() override {
        GL_Texture::bind();
        glCullFace(GL_FRONT);
    }
    void unbind() {
        glCullFace(GL_BACK);
    }
};

// interface for loading cubemap
class CubeMapLoader {
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
    CubeMapLoader() : cube_entity_(MeshFactory::get().get_mesh_entity(DefMeshList::CUBE)) {}

    virtual void init(const std::string& dir_path, bool flip = false) = 0;
};

class GL_CubeMapEntity : public GL_CubeMapTex, public CubeMapLoader {
    // decode the face into the gl equivelant
    static uint32_t gl_decode_face(CubeMapFace face);
    // parse and decode the full path name into the gl equivelant
    uint32_t gl_decode_face(const std::string& path_name);

public:
    GL_CubeMapEntity(const std::string& dir_path, bool flip = false) { init(dir_path, flip); }
    GL_CubeMapEntity() {}

    void init(const std::string& dir_path, bool flip = false) override;
    void draw(ShaderProgram& programs);
};

class Def_GL_CubeMapEntity : public GL_CubeMapEntity {
public:
    using GL_CubeMapEntity::GL_CubeMapEntity;

    Def_GL_CubeMapEntity() : GL_CubeMapEntity(DEF_CUBE_MAP_DIR_PATH) {}
};