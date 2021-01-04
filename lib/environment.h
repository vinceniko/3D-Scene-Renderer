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

public:
    std::unique_ptr<GL_CubeMapEntity> cube_map_;  // static env map
    GL_CubeMap_FBO cubemap_fbo_;  // dynamic env map

    DirLight dir_light_;
    PointLights point_lights_;
    
    GLCamera camera;
    
    GL_Depth_FBO depth_fbo_;
    bool debug_depth_map_ = false;
    DebugShadows debug_shadows_;

    Environment(std::unique_ptr<Camera> new_cam, int width, int height, PointLights&& point_lights, std::unique_ptr<GL_CubeMapEntity> cube_map) : camera(std::move(new_cam)), point_lights_(std::move(point_lights)), cube_map_(std::move(cube_map)), depth_fbo_(1024, 1024), width_(width), height_(height), cubemap_fbo_(width / 2.f) { set_viewport(width, height); }
    Environment(std::unique_ptr<Camera> new_cam, int width, int height, float fov, PointLights&& point_lights, std::unique_ptr<GL_CubeMapEntity> cube_map) : Environment(std::move(new_cam), width, height, std::move(point_lights), std::move(cube_map)) { 
        fov_ = fov;
        set_viewport(width, height); 
    }

    void bind_static();
    void bind_dynamic();

    void buffer(ShaderProgram& program);
    void buffer_lights(ShaderProgram& program);
    void buffer_shadows(ShaderProgram& program);
    void draw_lights(ShaderProgram& program);
    void draw_shadows(ShaderProgramCtx& programs, MeshEntityList& mesh_list);

    void draw_static_scene(ShaderProgramCtx& programs);
    void draw_static_cubemap(ShaderProgramCtx& programs);
    void draw_dynamic_cubemap(ShaderProgramCtx& programs, MeshEntity& mesh_entity, MeshEntityList& mesh_entities, std::function<void(MeshEntity&)> draw_f);

    void set_width(int width);
    void set_height(int height);
    void set_viewport(int width, int height);
    void reset_viewport();

    void set_cube_map(std::unique_ptr<GL_CubeMapEntity> cube_map);
    void swap_cube_map(std::unique_ptr<GL_CubeMapEntity>& cube_map);

    void draw_depth_map(ShaderProgramCtx& programs);
    void set_debug_depth_map(bool state);
    bool get_debug_depth_map();
};