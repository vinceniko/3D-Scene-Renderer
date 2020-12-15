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

    GL_CubeMap_FBO cubemap_fbo_;
    std::unique_ptr<GL_CubeMapEntity> cube_map_;
public:
    GL_Depth_FBO depth_fbo_;
    DirLight dir_light_;
    PointLights point_lights_;
    GLCamera camera;

    Environment(std::unique_ptr<Camera> new_cam, int width, int height, PointLights&& point_lights, std::unique_ptr<GL_CubeMapEntity> cube_map) : camera(std::move(new_cam)), point_lights_(point_lights), cube_map_(std::move(cube_map)), depth_fbo_(1024, 1024), width_(width), height_(height), cubemap_fbo_(width / 2.f) { set_viewport(width, height); }
    Environment(std::unique_ptr<Camera> new_cam, int width, int height, float fov, PointLights&& point_lights, std::unique_ptr<GL_CubeMapEntity> cube_map) : Environment(std::move(new_cam), width, height, std::move(point_lights), std::move(cube_map)) { 
        fov_ = fov;
        set_viewport(width, height); 
    }

    void bind_static() {
        cube_map_->bind();
    }
    void bind_dynamic() {
        cubemap_fbo_.bind();
    }

    void buffer(ShaderProgram& program) {
        camera.buffer(program);
        try {
            dir_light_.buffer_shadows(program);
            buffer_lights(program);
        } catch (const std::runtime_error& e) {
            // doing nothing is acceptable here, shader doesn't have the appropriate light uniform
#ifdef DEBUG
            // std::cout << "Light Error: " << e.what() << std::endl;
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
    void draw_shadows(ShaderProgramCtx& programs, MeshEntityList mesh_list) {
        programs.bind(ShaderPrograms::SHADOWS);
        depth_fbo_.bind();
        dir_light_.buffer_shadows(programs.get_selected_program());
        glDisable(GL_CULL_FACE);
        // glCullFace(GL_FRONT);
        for (MeshEntity& mesh : mesh_list) {
            mesh.draw_no_color(programs.get_selected_program()); 
        }
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        depth_fbo_.unbind();
        reset_viewport();
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
    void reset_viewport() {
        glViewport(0, 0, width_, height_);
    }

    void set_cube_map(std::unique_ptr<GL_CubeMapEntity> cube_map) {
        cube_map_ = std::move(cube_map);
    }
    void swap_cube_map(std::unique_ptr<GL_CubeMapEntity>& cube_map) {
        std::swap(cube_map_, cube_map);
    }
};