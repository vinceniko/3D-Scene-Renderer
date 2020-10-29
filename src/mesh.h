#include <fstream>
#include <string>
#include <vector>
#include <array>

#include <glm/vec3.hpp> // glm::vec3

#include <iostream>

typedef int unsigned uint;

class Mesh {
    using Indexer = std::vector<int>;

    std::vector<glm::vec3> verts_;
    std::vector<Indexer> faces_;

public:
    // file path constructor
    Mesh(std::string f_path);
    
    // accessors
    void push_back(glm::vec3 vert, Indexer indexer);
    void print();
};

class BumpyCubeMesh : Mesh {
public:
    BumpyCubeMesh() : Mesh("data/bump_cube.off") {}
};

class BunnyMesh : Mesh {
public:
    BunnyMesh() : Mesh("data/bunny.off") {}
};