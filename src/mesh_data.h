#include <string>

#include "mesh.h"

const std::string MESH_DIR = "../data/";

enum MeshList {
    CUBE,
    BUMPY,
    BUNNY,
};

class BumpyCubeMesh : public Mesh {
public:
    BumpyCubeMesh() : Mesh(MESH_DIR + "bumpy_cube.off") {}
};

class BunnyMesh : public Mesh {
public:
    BunnyMesh() : Mesh(MESH_DIR + "bunny.off") {}
};

class UnitCube : public Mesh {
public:
    UnitCube() : Mesh(MESH_DIR + "cube.off") {}
};