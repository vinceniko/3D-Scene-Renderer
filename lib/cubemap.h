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
class CubeMapTex : public Texture {
public:
    CubeMapTex() : Texture(GL_TEXTURE_CUBE_MAP) { init(); }
    CubeMapTex(int width) : Texture(GL_TEXTURE_CUBE_MAP, width, width) { init(); }
    CubeMapTex(int width, int height) : Texture(GL_TEXTURE_CUBE_MAP, width, height) { init(); }

    // create an empty cubemap
    void init() override;
    void bind() override {
        bind(GL_TEXTURE0);
    }
    void bind(uint32_t tex_unit) override {
        Texture::bind(tex_unit);
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

class CubeMapEntity : public CubeMapTex, public CubeMapLoader {
    // decode the face into the gl equivelant
    static uint32_t gl_decode_face(CubeMapFace face);
    // parse and decode the full path name into the gl equivelant
    uint32_t gl_decode_face(const std::string& path_name);

public:
    CubeMapEntity(const std::string& dir_path, bool flip = false) { init(dir_path, flip); }
    CubeMapEntity() {}

    void init(const std::string& dir_path, bool flip = false) override;
    void draw();
};

class Def_CubeMapEntity : public CubeMapEntity {
public:
    using CubeMapEntity::CubeMapEntity;

    Def_CubeMapEntity() : CubeMapEntity(DEF_CUBE_MAP_DIR_PATH) {}
};