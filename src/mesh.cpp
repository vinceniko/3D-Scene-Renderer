#include "mesh.h"

Mesh::Mesh(std::string f_path) {
    std::ifstream f(f_path);

    // TODO: throw error
    if (!f.is_open()) {
        std::cout << "Error opening file" << std::endl;
        exit(1);
    }

    // local scope to destroy `OFF` string
    {
        const int off_size = 3;
        char optional_off[off_size+1]; optional_off[off_size] = '\0';
        f.read(optional_off, off_size);

        // TODO: throw error
        if (f.bad()) {
            std::cout << "Error reading file" << std::endl;
            exit(1);
        }

        // First line (optional): the letters OFF to mark the file type.
        if (strcmp(optional_off, "OFF") != 0) { // did not start with off
            f.seekg(0);
        }
    }

    // Second line: the number of vertices, number of faces, and number of edges, in order (the latter can be ignored).
    {
        uint n_verts, n_faces, n_edges;
        f >> n_verts >> n_faces >> n_edges;

        // init buffers
        verts_.reserve(n_verts);
        faces_.reserve(n_faces);
    }

    // push verts
    glm::vec3 vert;
    for (size_t i = 0; i < verts_.capacity(); i++) {
        f >> vert.x >> vert.y >> vert.z;
        verts_.push_back(vert);
    }

    // number of vertices for the face
    uint n_verts_face;
    // push indices
    for (size_t i = 0; i < faces_.capacity(); i++) {
        f >> n_verts_face;
        // indexes of the composing vertices
        Indexer indexer(n_verts_face);
        for (size_t i = 0; i < n_verts_face; i++) {
            f >> indexer[i];
        }
        faces_.push_back(indexer);
    }
}

void Mesh::push_back(glm::vec3 vert, Indexer indexer) {
    verts_.push_back(vert);
    faces_.push_back(indexer);
}
void Mesh::print() {
    std::cout << verts_.size() << ' ' << faces_.size() << ' ' << ' ' << 0 << std::endl;
    for (auto vert : verts_) {
        std::cout << vert.x << ' ' << vert.y << ' ' << vert.z << std::endl;
    }
    for (auto indices : faces_) {
        std::cout << indices.size() << ' ';
        for (auto val : indices) {
            std::cout << val << ' ';
        }
        std::cout << std::endl;
    }
}
