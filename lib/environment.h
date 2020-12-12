#pragma once

#include "shader.h"
#include "framebuffer.h"
#include "cubemap.h"
#include "camera.h"
#include "light.h"

class Environment {
    float fov_ = 50.0;

    int width_;
    int height_;

    GL_CubeMap_FBO fbo;
    std::unique_ptr<GL_CubeMapEntity> cube_map_;
public:
    DirLight dir_light_;
    PointLights point_lights_{ PointLight{ glm::vec3(1.5f, 1.f, 0.f) } };
    GLCamera camera;

    Environment(std::unique_ptr<Camera> new_cam, int width, int height) : cube_map_(std::make_unique<GL_CubeMapEntity>()), fbo(width_ / 2.f), width_(width), height_(height), camera(std::move(new_cam)) { set_viewport(width, height); }
    Environment(std::unique_ptr<Camera> new_cam, int width, int height, float fov) : Environment(std::move(new_cam), width, height) { 
        fov_ = fov;
        set_viewport(width, height); 
    }
    Environment(std::unique_ptr<Camera> new_cam, int width, int height, std::unique_ptr<GL_CubeMapEntity> cube_map) : camera(std::move(new_cam)), cube_map_(std::move(cube_map)), fbo(width_ / 2.f), width_(width), height_(height) { set_viewport(width, height); }
    Environment(std::unique_ptr<Camera> new_cam, int width, int height, float fov, std::unique_ptr<GL_CubeMapEntity> cube_map) : Environment(std::move(new_cam), width, height, std::move(cube_map)) { 
        fov_ = fov;
        set_viewport(width, height); 
    }

    void bind_static() {
        cube_map_->bind();
    }
    void bind_dynamic() {
        fbo.bind();
    }

    void buffer(ShaderProgram& program) {
        camera.buffer(program);
        try {
            buffer_lights(program);
        } catch (const std::runtime_error& e) {
            // doing nothing is acceptable here, shader doesn't have the appropriate light uniform
#ifdef DEBUG
            std::cout << "Light Error: " << e.what() << std::endl;
#endif
        }
    }
    void buffer_lights(ShaderProgram& program) {
        dir_light_.buffer(program);
        point_lights_.buffer(program);
    }
    void draw_lights(ShaderProgram& program) {
        buffer(program);
        point_lights_.draw(program);
    }

    void draw_static(ShaderProgramCtx& programs);
    // draw the scene to a the fbo
    void draw_dynamic(ShaderProgramCtx& programs, MeshEntity& mesh_entity, MeshEntityList& mesh_entities, std::function<void(MeshEntity&)> draw_f);

    void set_width(int width) {
        width_ = width;
    }
    void set_height(int height) {
        height_ = height;
    }
    void set_viewport(int width, int height) {
        width_ = width;
        height_ = height;
        glViewport(0, 0, width, height);
    }

    void set_cube_map(std::unique_ptr<GL_CubeMapEntity> cube_map) {
        cube_map_ = std::move(cube_map);
    }
    void swap_cube_map(std::unique_ptr<GL_CubeMapEntity>& cube_map) {
        std::swap(cube_map_, cube_map);
    }
};