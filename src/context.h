#pragma once

#include "helpers.h"

#include "camera.h"
#include "mesh.h"

class GLContext {
    Program& program_;

public:
    friend MeshEntity;

    GLCamera camera;
    GLMeshCtx mesh_ctx;
    MeshEntityList mesh_list;

    GLContext(Program& program) : program_(program), camera(program_), mesh_ctx(program_), mesh_list() {}
    GLContext(Program& program, GLCamera camera) : program_(program), camera(camera), mesh_ctx(program_), mesh_list() {}
};