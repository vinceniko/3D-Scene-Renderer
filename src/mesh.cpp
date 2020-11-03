#include "mesh.h"

Mesh::Mesh(std::string f_path) {
    std::ifstream f(f_path);

    // TODO: throw error
    if (!f.is_open()) {
        throw std::runtime_error("Error opening file");
    }

    // local scope to destroy `OFF` string
    {
        const int off_size = 3;
        char optional_off[off_size+1]; optional_off[off_size] = '\0';
        f.read(optional_off, off_size);

        // TODO: throw error
        if (f.bad()) {
            throw std::runtime_error("Error reading file");
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
        if (static_cast<unsigned long>(n_verts_face) != TRI) {
            throw std::runtime_error("Error: Not A Triangle Mesh");
        }
        // indexes of the composing vertices
        Indexer indexer;
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
void Mesh::print() const {
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

const size_t MeshEntity::get_id() const {
    return id_;
}

MeshEntity::MeshEntity(GLMeshCtx& ctx, size_t id, const GLMesh& mesh) : 
ctx_(ctx), id_(id), model_uniform_(ctx.program_, "model_trans", model_trans_), color_(glm::vec3(0.0, 0.0, 1.0)) {
    // model_trans_ = glm::translate(model_trans_, glm::vec3(1.f, 1.f, -2.f));
}


bool MeshEntity::intersected_triangles(glm::vec3 world_ray_origin, glm::vec3 world_ray_dir) {
    const GLMesh& mesh = ctx_.get_meshes()[id_];

    glm::vec3 model_ray_origin = glm::inverse(model_trans_) * glm::vec4(world_ray_origin, 1.f);
    glm::vec3 model_ray_dir = glm::inverse(model_trans_) * glm::vec4(world_ray_dir, 0.f);

    std::cout << "world_ray_origin: " << world_ray_origin[0] << ' ' << world_ray_origin[1] << ' ' << world_ray_origin[2] << std::endl;
    std::cout << "world_ray_dir: " << world_ray_dir[0] << ' ' << world_ray_dir[1] << ' ' << world_ray_dir[2] << std::endl;
    std::cout << "model_ray_origin: " << model_ray_origin[0] << ' ' << model_ray_origin[1] << ' ' << model_ray_origin[2] << std::endl;
    std::cout << "model_ray_dir: " << model_ray_dir[0] << ' ' << model_ray_dir[1] << ' ' << model_ray_dir[2] << std::endl;

    for (const Indexer& face : mesh.get_faces()) {
        std::array<glm::vec3, TRI> tri{mesh.get_verts()[face[0]], mesh.get_verts()[face[1]], mesh.get_verts()[face[2]]};
        // std::cout << "mesh: " << tri[0][0] << ' ' << tri[0][1] << ' ' << tri[0][2] << std::endl;
        // std::cout << "mesh: " << tri[1][0] << ' ' << tri[1][1] << ' ' << tri[1][2] << std::endl;
        // std::cout << "mesh: " << tri[2][0] << ' ' << tri[2][1] << ' ' << tri[2][2] << std::endl;


        glm::vec2 bary_pos;
        float distance;
        if (glm::intersectRayTriangle(model_ray_origin, model_ray_dir, tri[0], tri[1], tri[2], bary_pos, distance)) {
            return true;
        }
    }
    return false;
}

void MeshEntity::draw() {
    const GLMesh& mesh_ref = ctx_.get_meshes()[id_];
    glBindVertexArray(mesh_ref.VAO_);

    model_uniform_.buffer(model_trans_);

    glUniform3f(ctx_.program_.uniform("triangle_color"), color_.r, color_.g, color_.b);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL );
    glDrawElements(GL_TRIANGLES, mesh_ref.get_faces().size() * TRI, GL_UNSIGNED_INT, 0);

    glUniform3f(ctx_.program_.uniform("triangle_color"), 1.f - color_.r, 1.f - color_.g, 1.f - color_.b);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDrawElements(GL_TRIANGLES, mesh_ref.get_faces().size() * TRI, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    #ifdef DEBUG
        check_gl_error();
    #endif
}

MeshEntityList GLMeshCtx::push(std::vector<Mesh> meshes) {
    // gen gl objects
    std::vector<uint> VAOs(meshes.size()), VBOs(meshes.size()), EBOs(meshes.size());
    glGenVertexArrays(meshes.size(), VAOs.data());
    glGenBuffers(meshes.size(), VBOs.data());
    glGenBuffers(meshes.size(), EBOs.data());

    // return prototype entities
    MeshEntityList out;
    out.reserve(meshes.size());

    for (uint i = 0; i < meshes.size(); i++) {
        // assign gl objects and commit to mesh list
        meshes_.push_back(GLMesh{VAOs[i], VBOs[i], EBOs[i], std::move(meshes[i])});
        const GLMesh& inserted_mesh = meshes_[meshes_.size() - 1];

        // bind to VAO
        glBindVertexArray(VAOs[i]);
        // buffer data to VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * inserted_mesh.get_verts().size(), inserted_mesh.get_verts().data(), GL_STATIC_DRAW);
        // buffer data to EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * TRI * inserted_mesh.get_faces().size(), inserted_mesh.get_faces().data(), GL_STATIC_DRAW);
        
        // vertex positions
        int position_id = program_.attrib("position");
        if (position_id < 0) {
            throw std::runtime_error("gl vertex attribute not found");
        }
        glEnableVertexAttribArray(position_id);
        glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // TODO: add calculated norms

        // unbind VAO
        glBindVertexArray(0);

        #ifdef DEBUG
            check_gl_error();
        #endif

        out.push_back(MeshEntity{*this, i, meshes_[meshes_.size() - 1]});
    }

    return out;
};
MeshEntity GLMeshCtx::push(Mesh mesh) {
    // gen gl objects
    uint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // assign gl object
    meshes_.push_back(GLMesh{VAO, VBO, EBO, mesh});

    // return prototype entity
    return MeshEntity{*this, meshes_.size() - 1, meshes_[meshes_.size() - 1]};
};

const std::vector<GLMesh>& GLMeshCtx::get_meshes() const {
    return meshes_;
}