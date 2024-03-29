#pragma once

#include <array>

#include "glm/vec2.hpp"

#include "camera.h"
#include "mesh.h"

#include "environment.h"

#ifdef DEBUG
#include <iostream>
#endif

// state of the mouse
class MouseContext {
    int selected_ = -1;
    bool held_ = false;

    glm::vec2 pos_{ 0.f };
    glm::vec2 prev_world_point_{ 1.f };

    double prev_scroll_{ 0.f };
    double scroll_{ 0.f };

public:
    // hold without selecting
    void hold();
    // release a hold
    void release();
    // hold and select
    void hold_select(size_t i);
    // deselect only
    void deselect();

    int get_selected() const;
    void set_selected(int selected);
    bool is_selected() const;
    bool is_held() const;

    void set_position(glm::vec2 world_point);
    glm::vec2 get_position() const;
    glm::vec2 get_prev_position() const;

    void set_scroll(double scroll);
    double get_scroll() const;
    double get_prev_scroll() const;
};

// general context, holds all other state
class Context {
public:
    int base_width_;
    int base_height_;

    std::unique_ptr<DefRenderer> renderer;
    std::unique_ptr<DefMeshFactory> mesh_factory;

    MeshEntityList mesh_list;

    std::unique_ptr<Environment> env;

    MouseContext mouse_ctx;

    std::unique_ptr<FBO> main_fbo_;
    std::unique_ptr<Offscreen_FBO> offscreen_fbo_;
    std::unique_ptr<Offscreen_FBO_Multisample> offscreen_fbo_msaa_;

    bool msaa_use_ = false;
    bool fxaa_use_ = true;
    
    std::unique_ptr<Depth_FBO> depth_fbo_;

    bool draw_grid_ = true;
    bool debug_depth_map_ = false;
    std::unique_ptr<DebugShadows> debug_shadows_;

    Context(int width, int height, int base_width, int base_height);

    // tests whether a ray in world space intersected with a mesh stored in mesh_list
    int intersected_mesh_perspective(glm::vec3 world_ray) const;
    // tests whether a ray in world space intersected with a mesh stored in mesh_list
    int intersected_mesh_ortho(glm::vec3 world_pos) const;
    // mutates mouse state in mouse_ctx if a mesh is intersected
    void select(glm::vec2 cursor_pos, float width, float height);
    // mutates mouse state in mouse_ctx if a mesh is intersected with a world space ray
    void select_perspective(glm::vec3 world_ray);
    // mutates mouse state in mouse_ctx if a mesh is intersected with a world space pos
    void select_ortho(glm::vec3 world_pos);
    // mutates mouse state in mouse_ctx to deselect a selected mesh
    void deselect();
    // swap the selected mesh with another mesh in the mesh list, used to draw selected mesh last
    void swap_selected_mesh(const uint32_t idx);
    // returns an optional selected mesh
    Optional<MeshEntity> get_selected();

    // loads in mesh prototypes into the mesh_factory
    void init_mesh_prototypes(std::vector<Mesh>&& meshes);
    // adds a mesh entity to mesh_list. these are references to the prototypes in mesh_factory
    void push_mesh_entity(std::vector<int>&& ids);

    // frame by frame updates. call prior to drawing
    void update(std::chrono::duration<float> delta);
    // updates and draws the model using the user bound shader program and the selected draw mode
    void draw();
    void draw_selected_to_stencil(MeshEntity& mesh_entity);
    void draw_selected(MeshEntity& mesh_entity);
    void draw_grid();
    void draw_static(MeshEntity& mesh_entity);
    // draws a model based on its mode
    void draw_w_mode(MeshEntity& mesh_entity);
    // draws a model when other model's state's are also necessary; such as dynamic reflections
    void draw(FBO& main_fbo, MeshEntity& mesh_entity, MeshEntityList& mesh_entity_list);
    // draws the model using the user bound shader program
    void draw_surfaces(MeshEntity& mesh_entity);
    // draws a wireframe above the mesh
    void draw_wireframe(MeshEntity& mesh_entity);
    // draws the mesh normals
    void draw_normals(MeshEntity& mesh_entity);
    // updates and draws the models using the user bound shader program and the selected draw mode
    void draw_surfaces();
    // draws a wireframe above the meshes
    void draw_wireframes();
    // draws the mesh normals
    void draw_normals();
    void draw_depth_map();
    void draw_offscreen(Offscreen_FBO& draw_fbo);
    void draw_fxaa(Offscreen_FBO& draw_fbo);

    void set_viewport(int width, int height);
    void set_env(std::unique_ptr<Environment>&& env);
};