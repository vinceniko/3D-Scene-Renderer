#pragma once

#include <string>

#include "mesh.h"

const std::string MESH_DIR = "../data/";

enum MeshList {
    BUMPY,
    BUNNY,
    MONKEY,
};

class BumpyCubeMesh : public Mesh {
public:
    BumpyCubeMesh() : Mesh(MESH_DIR + "bumpy_cube.off") {}
};

class BunnyMesh : public Mesh {
public:
    BunnyMesh() : Mesh(MESH_DIR + "bunny.off") {}
};

class MonkeyMesh : public Mesh {
public:
    MonkeyMesh() : Mesh(MESH_DIR + "monkey.off") {}
};
