#pragma once

#include "helpers.h"

#include <string>
#include <filesystem>

#include "shader.h"
#include "mesh.h"
#include "camera.h"

const std::string CUBE_MAP_DIR_PATH = "../data/night_env/";

class CubeMap {
    enum CubeMapFace {
        FRONT,
        BACK,
        LEFT,
        RIGHT,
        TOP,
        BOTTOM
    };

    static CubeMapFace decode_face(const std::string& kind) {
        if (kind == "posz") {
            return CubeMapFace::BACK;
        }
        else if (kind == "negz") {
            return CubeMapFace::FRONT;
        }
        else if (kind == "negx") {
            return CubeMapFace::LEFT;
        }
        else if (kind == "posx") {
            return CubeMapFace::RIGHT;
        }
        else if (kind == "posy") {
            return CubeMapFace::TOP;
        }
        else if (kind == "negy") {
            return CubeMapFace::BOTTOM;
        }
        throw std::runtime_error("Error decoding CubeMap face: " + kind);
    }

    static uint32_t gl_decode_face(CubeMapFace face) {
        switch (face) {
        case BACK:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case FRONT:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        case LEFT:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case RIGHT:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case TOP:
            return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case BOTTOM:
            return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        }
        throw std::runtime_error("Error translating CubeMap face to GL face");
    }

public:
    virtual void load(const std::string& dir_path) = 0;

    static CubeMapFace parse_path_name(const std::string& path_name) {
        // index of extension
        size_t ext_idx = path_name.rfind('.');
        if (ext_idx == std::string::npos) {
            throw "Error finding extension name of file";
        }
        // ex: "negex"
        constexpr int face_size = 4;

        std::string face_str = path_name.substr(ext_idx - face_size, face_size);

        std::cout << "face_str " << face_str << std::endl; 

        return decode_face(face_str);
    }

    uint32_t get_face(const std::string& path_name) {
        return gl_decode_face(parse_path_name(path_name));
    }
};

class GLCubeMap : public CubeMap {
    std::reference_wrapper<ShaderProgramCtx> programs_;
    
    MeshEntity cube_entity_;

    uint32_t tex_id_;

public:
    GLCubeMap(ShaderProgramCtx& programs, MeshFactory& mesh_factory, const std::string& dir_path) : programs_(programs), cube_entity_(mesh_factory.get_mesh_entity(DefMeshList::CUBE)) {
        cube_entity_.scale(glm::mat4(1.f), Spatial::ScaleDir::Out, 2.0);
        load(dir_path);
    }
    GLCubeMap(ShaderProgramCtx& programs, MeshFactory& mesh_factory) : GLCubeMap(programs, mesh_factory, CUBE_MAP_DIR_PATH) {}

    void load(const std::string& dir_path) override;

    void draw() {
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id_);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        glDepthMask(GL_FALSE);

        cube_entity_.draw_no_color();
        glDepthMask(GL_TRUE);
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};

class Environment {
    ShaderProgramCtx& programs_;
    GLCubeMap cube_map_;

    float fov_ = 90.0;

public:
    GLCamera camera;

    Environment(ShaderProgramCtx& programs, MeshFactory& mesh_factory, std::shared_ptr<Camera> new_cam) : programs_(programs), cube_map_(programs, mesh_factory), camera(programs, new_cam) {}
    Environment(ShaderProgramCtx& programs, MeshFactory& mesh_factory, std::shared_ptr<Camera> new_cam, float fov) : programs_(programs), cube_map_(programs, mesh_factory), camera(programs, new_cam), fov_(fov) {}

    void draw() {
        ShaderPrograms selected = programs_.get_selected();
        programs_.bind(ShaderPrograms::ENV);

        glm::mat4 old_camera_view = camera->get_view();
        Camera::Projection old_mode = camera->get_projection_mode();
        float old_fov = camera->get_fov();
        camera->set_view(glm::mat3(old_camera_view));
        camera->set_projection_mode(Camera::Projection::Perspective);
        camera->set_fov(fov_);
        
        camera.buffer();
        
        cube_map_.draw();

        camera->set_fov(old_fov);
        camera->set_view(old_camera_view);
        camera->set_projection_mode(old_mode);
        programs_.bind(selected);
    }
};