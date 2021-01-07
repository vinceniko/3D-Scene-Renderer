#pragma once

#include "shader.h"
#include "framebuffer.h"
#include "cubemap.h"
#include "camera.h"
#include "light.h"

class Environment {
    float fov_ = 50.0;

public:
    std::unique_ptr<GL_CubeMapEntity> cube_map_;  // static env map
    GL_CubeMap_FBO cubemap_fbo_;  // dynamic env map

    DirLight dir_light_;
    PointLights point_lights_;
    
    GLCamera camera;
    
    Environment(std::unique_ptr<Camera> new_cam, int width, PointLights&& point_lights, std::unique_ptr<GL_CubeMapEntity> cube_map) : Environment(std::move(new_cam), width, 50.0, std::move(point_lights), std::move(cube_map))  {}
    Environment(std::unique_ptr<Camera> new_cam, int width, float fov, PointLights&& point_lights, std::unique_ptr<GL_CubeMapEntity> cube_map) : camera(std::move(new_cam)), point_lights_(std::move(point_lights)), cube_map_(std::move(cube_map)), cubemap_fbo_(width / 2.f), fov_(fov) {}

    void bind_static();
    void bind_dynamic();

    void buffer(ShaderProgram& program);
    void buffer_lights(ShaderProgram& program);
    void buffer_shadows(ShaderProgram& program);
    void draw_lights(ShaderProgram& program);
    void draw_shadows(ShaderProgramCtx& programs, GL_FBO_RBO_Interface& main, MeshEntityList& mesh_list);

    void draw_static_scene(ShaderProgramCtx& programs);
    void draw_static_cubemap(ShaderProgramCtx& programs);
    void draw_dynamic_cubemap(ShaderProgramCtx& programs, GL_FBO_RBO_Interface& main_fbo, MeshEntity& mesh_entity, MeshEntityList& mesh_entities, std::function<void(MeshEntity&)> draw_f);

    void set_cube_map(std::unique_ptr<GL_CubeMapEntity> cube_map);
    void swap_cube_map(std::unique_ptr<GL_CubeMapEntity>& cube_map);
};