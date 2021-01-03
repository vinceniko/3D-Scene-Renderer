#include "mesh.h"

#include <cstring>

Mesh::Mesh(std::string f_path) {
    std::ifstream f(f_path);

    // TODO: throw error
    if (!f.is_open()) {
        throw std::runtime_error("Error opening file");
    }

    // local scope to destroy `OFF` string
    {
        const int off_size = 3;
        char optional_off[off_size + 1]; optional_off[off_size] = '\0';
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

    init();
}

void Mesh::init() {
    centroid_ = calc_centroid();
    scale_ = calc_scale();
    normals_ = calc_normals();
}

void Mesh::push_back(glm::vec3 vert, const Indexer& indexer) {
    verts_.push_back(vert);
    faces_.push_back(indexer);
}

const std::vector<glm::vec3>& Mesh::get_verts() const {
    return verts_;
}
const std::vector<glm::vec3>& Mesh::get_normals() const {
    return normals_;
}
const std::vector<Indexer>& Mesh::get_faces() const {
    return faces_;
}
const glm::vec3& Mesh::get_centroid() const {
    return centroid_;
}
const glm::vec3& Mesh::get_scale() const {
    return scale_;
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

std::vector<glm::vec3> Mesh::calc_normals() const {
    std::vector<glm::vec3> normals{ verts_.size(), glm::vec3{0.0} };
    for (const Indexer& face : get_faces()) {
        Triangle tri{ get_verts()[face[0]], get_verts()[face[1]], get_verts()[face[2]] };

        glm::vec3 normal = glm::cross(tri[1] - tri[0], tri[2] - tri[0]);

        normals[face[0]] += normal;
        normals[face[1]] += normal;
        normals[face[2]] += normal;
    }
    for (glm::vec3& normal : normals) {
        normal = glm::normalize(normal);
    }

    return normals;
}

glm::vec3 Mesh::calc_centroid() const {
    glm::vec3 mg{ 0.f };
    float m = 0.f;

    for (Indexer face : get_faces()) {
        Triangle tri{ get_verts()[face[0]], get_verts()[face[1]], get_verts()[face[2]] };

        float tri_area = area(tri);
        m += tri_area;
        mg += tri_area * centroid(tri);
    }
    glm::vec3 out = mg / m;
    // #ifdef DEBUG
    // std::cout << "m: " << m << ' '
    //     << "mg: " << mg[0] << ' ' << mg[1] << ' ' << mg[2] << ' ' << std::endl
    //     << "out: " << out[0] << ' ' << out[1] << ' ' << out[2] << ' ' << std::endl;
    // #endif

    return out;
}
glm::vec3 Mesh::calc_scale() const {
    float xmin = std::numeric_limits<float>::infinity();
    float xmax = -std::numeric_limits<float>::infinity();
    float ymin = std::numeric_limits<float>::infinity();
    float ymax = -std::numeric_limits<float>::infinity();
    float zmin = std::numeric_limits<float>::infinity();
    float zmax = -std::numeric_limits<float>::infinity();
    for (glm::vec3 pos : get_verts()) {
        xmin = std::min(xmin, pos.x);
        xmax = std::max(xmax, pos.x);

        ymin = std::min(ymin, pos.y);
        ymax = std::max(ymax, pos.y);

        zmin = std::min(zmin, pos.z);
        zmax = std::max(zmax, pos.z);
    }

    float x_dist = xmax - xmin;
    float y_dist = ymax - ymin;
    float z_dist = zmax - zmin;
    auto dists = std::array<float, 3>{ x_dist, y_dist, z_dist };
    float max_dist = *std::max_element(dists.begin(), dists.end());

    return glm::vec3{ 1.f / max_dist };
}

void GLMesh::init(ShaderProgram& program, int VAO, uint32_t VBO, uint32_t EBO) {
    // bind to VAO
    glBindVertexArray(VAO);
    // buffer data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    size_t size_verts = sizeof(glm::vec3) * get_verts().size();
    size_t size_normals = sizeof(glm::vec3) * get_verts().size();
    glBufferData(GL_ARRAY_BUFFER, size_verts + size_normals, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size_verts, get_verts().data());
    glBufferSubData(GL_ARRAY_BUFFER, size_verts, size_normals, get_normals().data());
    // buffer data to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * TRI * get_faces().size(), get_faces().data(), GL_STATIC_DRAW);

    // vertex positions
    int32_t position_id = program.attrib("a_pos");
    if (position_id < 0) {
        throw std::runtime_error("gl vertex attribute not found: a_pos");
    }
    glEnableVertexAttribArray(position_id);

    int32_t normal_id = program.attrib("a_normal");
    if (normal_id < 0) {
        throw std::runtime_error("gl vertex attribute not found: a_normal");
    }
    glEnableVertexAttribArray(normal_id);

    glVertexAttribPointer(position_id, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(normal_id, 3, GL_FLOAT, GL_FALSE, 0, (void*)(size_verts));

    // unbind VA
}

const size_t MeshEntity::get_id() const {
    return id_;
}

MeshEntity::MeshEntity(MeshFactory& ctx, size_t id) :
    ctx_(ctx), id_(id), color_(glm::vec3(0.0, 0.0, 1.0)) {
    // u_model_trans_ = glm::translate(u_model_trans_, glm::vec3(1.f, 1.f, -2.f));
    // scale_to_unit();
    set_to_origin();
}

void MeshEntity::set_to_origin() {
    glm::mat4 scale = glm::scale(glm::mat4{ 1.f }, ctx_.get().get_meshes()[id_]->get_scale());
    glm::mat4 trans = glm::translate(glm::mat4{ 1.f }, -ctx_.get().get_meshes()[id_]->get_centroid());
    trans_ = scale * trans;
}

glm::vec3 MeshEntity::get_origin() {
    return glm::vec3(trans_ * glm::vec4(glm::vec3(0.f), 1.f));
}

glm::vec3 MeshEntity::get_position() {
    return get_mesh().get_centroid();
}

void MeshEntity::set_color(glm::vec3 new_color) {
    color_ = new_color;
}
glm::vec3 MeshEntity::get_color() const {
    return color_;
}

float MeshEntity::intersected_triangles(glm::vec3 world_ray_origin, glm::vec3 world_ray_dir) const {
    const GLMesh& mesh = *ctx_.get().get_meshes()[id_];

    glm::vec3 model_ray_origin = glm::inverse(trans_) * glm::vec4(world_ray_origin, 1.f);
    glm::vec3 model_ray_dir = glm::inverse(trans_) * glm::vec4(world_ray_dir, 0.f);

    float min_dist = std::numeric_limits<float>::infinity();
    for (const Indexer& face : mesh.get_faces()) {
        std::array<glm::vec3, TRI> tri{ mesh.get_verts()[face[0]], mesh.get_verts()[face[1]], mesh.get_verts()[face[2]] };

        glm::vec2 bary_pos;
        float distance;
        if (glm::intersectRayTriangle(model_ray_origin, model_ray_dir, tri[0], tri[1], tri[2], bary_pos, distance)) {
            if (min_dist > distance) {
                min_dist = distance;
            }
        }
    }
    if (min_dist == std::numeric_limits<float>::infinity()) {
        return -1;
    }
    return min_dist;
}

void MeshEntity::buffer(ShaderProgram& program) {
    model_uniform_.buffer(program, trans_);
    glUniform3f(program.uniform("u_object_color"), color_.r, color_.g, color_.b);
}

void MeshEntity::draw(ShaderProgram& program) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    const GLMesh& mesh_ref = *ctx_.get().get_meshes()[id_];
    glBindVertexArray(mesh_ref.VAO_);

    buffer(program);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, mesh_ref.get_faces().size() * TRI, GL_UNSIGNED_INT, 0);

#ifdef DEBUG
    check_gl_error();
#endif
}

void MeshEntity::draw_minimal(ShaderProgram& program) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    const GLMesh& mesh_ref = *ctx_.get().get_meshes()[id_];

    glBindVertexArray(mesh_ref.VAO_);

    model_uniform_.buffer(program, trans_);

    glDrawElements(GL_TRIANGLES, mesh_ref.get_faces().size() * TRI, GL_UNSIGNED_INT, 0);

#ifdef DEBUG
    check_gl_error();
#endif
}

void MeshEntity::draw_wireframe(ShaderProgram& program) {
    glDisable(GL_CULL_FACE);

    const GLMesh& mesh_ref = *ctx_.get().get_meshes()[id_];

    glBindVertexArray(mesh_ref.VAO_);

    buffer(program);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // // glLineWidth doesn't work, maybe an Apple driver bug 
    // glLineWidth(2.f);
    glDrawElements(GL_TRIANGLES, mesh_ref.get_faces().size() * TRI, GL_UNSIGNED_INT, 0);

#ifdef DEBUG
    check_gl_error();
#endif
}

MeshEntityList MeshFactory::push(ShaderProgram& program, std::vector<Mesh> meshes) {
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
        meshes_.push_back(std::make_unique<GLMesh>(program, VAOs[i], VBOs[i], EBOs[i], std::move(meshes[i])));

#ifdef DEBUG
        check_gl_error();
#endif

        out.push_back(std::make_unique<MeshEntity>(get_mesh_entity(meshes_.size() - 1)));
    }

    return out;
};

const std::vector<std::unique_ptr<GLMesh>>& MeshFactory::get_meshes() const {
    return meshes_;
}

MeshEntity MeshFactory::get_mesh_entity(int i) {
    return MeshEntity{ *this, MeshFactory::get_from_kind(i) };
}
MeshEntity MeshFactory::get_mesh_entity(size_t i) {
    return MeshEntity{ *this, i };
}

void MeshEntityList::draw(ShaderProgram& program) {
    for (auto& mesh : *this) {
        mesh->draw(program);
    }
}
void MeshEntityList::draw_wireframes(ShaderProgram& program) {
    for (auto& mesh : *this) {
        mesh->draw_wireframe(program);
    }
}

const GLMesh& MeshEntity::get_mesh() {
    return *ctx_.get().get_meshes()[id_];
}
