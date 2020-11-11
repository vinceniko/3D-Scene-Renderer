#pragma once

#include <array>

#include "glm/vec2.hpp"

#include "helpers.h"

#include "camera.h"
#include "mesh.h"
#include "shader.h"

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
    // the shaders that the program will use
    std::array<ShaderPrograms, 2> shaders = { ShaderPrograms::PHONG, ShaderPrograms::FLAT };
    // an index into the list of program shaders
    size_t shader_idx = 0;

    // extra modes for drawing. mostly for debug purposes, such as wireframe.
    enum DrawMode {
        DEF,
        WIREFRAME,
        NORMALS,

        SIZE = 3,
    };

public:
    DrawMode draw_mode = DrawMode::DEF;

    std::shared_ptr<ShaderProgramCtx> programs;

    GLCamera camera;

    MouseContext mouse_ctx;

    MeshFactory mesh_factory;
    MeshEntityList mesh_list;

    Context(std::unique_ptr<ShaderProgramCtx> programs);
    Context(std::unique_ptr<ShaderProgramCtx> programs, std::shared_ptr<Camera> camera);

    // tests whether a ray in world space intersected with a mesh stored in mesh_list
    int intersected_mesh(glm::vec3 world_ray_dir) const;
    // mutates mouse state in mouse_ctx if a mesh is intersected with a world space ray
    void select(glm::vec3 world_ray_dir);
    // mutates mouse state in mouse_ctx to deselect a selected mesh
    void deselect();
    // returns an optional selected mesh
    Optional<MeshEntity> get_selected();

    // loads in mesh prototypes into the mesh_factory
    void init_mesh_prototypes(std::vector<Mesh> meshes);
    // adds a mesh entity to mesh_list. these are references to the prototypes in mesh_factory
    void push_mesh_entity(std::vector<size_t> ids);

    // cycles through the available draw modes enumerated in GLContext::DrawMode
    void switch_draw_mode();
    // cycles through the available shader programs enumerated in ProgramList
    void switch_program();

    // draws the models using the user bound shader program and the selected draw mode
    void draw();
    // draws the models using the user bound shader program
    void draw_surface();
    // draws a wireframe above the mesh
    void draw_wireframe();
    // draws the mesh normals
    void draw_normals();

    // frame by frame updates. call prior to drawing
    void update();
};