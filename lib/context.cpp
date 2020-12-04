#include <cmath>

#include "context.h"

void MouseContext::hold() {
    held_ = true;
}
void MouseContext::hold_select(size_t i) {
    selected_ = i;
    held_ = true;
}
void MouseContext::release() {
    held_ = false;
}
void MouseContext::deselect() {
    selected_ = -1;
}
int MouseContext::get_selected() const {
    return selected_;
}
bool MouseContext::is_selected() const {
    return selected_ >= 0;
}
bool MouseContext::is_held() const {
    return held_;
}
void MouseContext::set_position(glm::vec2 world_point) {
    prev_world_point_ = pos_;
    pos_ = world_point;
}
glm::vec2 MouseContext::get_position() const {
    return pos_;
}
glm::vec2 MouseContext::get_prev_position() const {
    return prev_world_point_;
}
void MouseContext::set_scroll(double scroll) {
    prev_scroll_ = scroll_;
    scroll_ = scroll;
}
double MouseContext::get_prev_scroll() const {
    return prev_scroll_;
}
double MouseContext::get_scroll() const {
    return scroll_;
}

Context::Context(std::unique_ptr<ShaderProgramCtx> programs, std::shared_ptr<Camera> new_cam, int width, int height) :
    programs(std::move(programs)), mesh_factory(this->programs->get_selected_program()), env(this->mesh_factory, new_cam, width, height) {}

int Context::intersected_mesh_perspective(glm::vec3 world_ray) const {
    float min_dist = std::numeric_limits<float>::infinity();
    int closest = -1;
    for (size_t i = 0; i < mesh_list.size(); i++) {
        float distance = mesh_list[i].intersected_triangles(env.camera->get_position(), world_ray);
        if (distance >= 0) {
            if (min_dist > distance) {
                min_dist = distance;
                closest = i;
            }
        }
    }
    return closest;
}
int Context::intersected_mesh_ortho(glm::vec3 world_pos) const {
    float min_dist = std::numeric_limits<float>::infinity();
    int closest = -1;
    for (size_t i = 0; i < mesh_list.size(); i++) {
        float distance = mesh_list[i].intersected_triangles(world_pos, glm::inverse(env.camera->get_view()) * glm::vec4(0.0, 0.0, -1.f, 0.f));
        if (distance >= 0) {
            if (min_dist > distance) {
                min_dist = distance;
                closest = i;
            }
        }
    }
    return closest;
}
void Context::select(glm::vec2 cursor_pos, float width, float height) {
    if (env.camera->get_projection_mode() == Camera::Projection::Ortho) {
        glm::vec3 pos_world = env.camera->get_pos_world(cursor_pos, width, height);
        select_ortho(pos_world);
    }
    else {
        glm::vec3 ray_world = env.camera->get_ray_world(cursor_pos, width, height);
        select_perspective(ray_world);
    }
}
void Context::select_perspective(glm::vec3 world_ray) {
    int found = intersected_mesh_perspective(world_ray);
    if (found >= 0 && found != mouse_ctx.get_selected()) {
        deselect();
        mouse_ctx.hold_select(found);
        mesh_list[found].set_color(glm::vec3(1.f) - mesh_list[found].get_color());
    }
    else if (found >= 0) {
        deselect();
    }
}
void Context::select_ortho(glm::vec3 world_pos) {
    int found = intersected_mesh_ortho(world_pos);
    if (found >= 0 && found != mouse_ctx.get_selected()) {
        deselect();
        mouse_ctx.hold_select(found);
        mesh_list[found].set_color(glm::vec3(1.f) - mesh_list[found].get_color());
    }
    else if (found >= 0) {
        deselect();
    }
}
void Context::deselect() {
    if (mouse_ctx.is_selected()) {
        mesh_list[mouse_ctx.get_selected()].set_color(glm::vec3(1.f) - mesh_list[mouse_ctx.get_selected()].get_color());
    }
    mouse_ctx.deselect();
}
Optional<MeshEntity> Context::get_selected() {
    if (mouse_ctx.is_selected()) {
        return { {mesh_list[mouse_ctx.get_selected()] } };
    }

    return {};
}

void Context::update() {
    // #ifdef DEBUG
    // std::cout << "new_point: " << new_point[0] << ' ' << new_point[1] << ' ' << 
    // "old_point: " << old_point[0] << ' ' << old_point[1] << ' ' << std::endl;
    // #endif

    if (mouse_ctx.is_held()) {
        glm::vec2 old_point = mouse_ctx.get_prev_position();
        glm::vec2 new_point = mouse_ctx.get_position();
#ifdef DEBUG
        auto diff = new_point - old_point;
        std::cout << "diff: " << diff[0] << ' ' << diff[1] << std::endl;
#endif
        env.camera->translate(glm::vec3(new_point, 0.f), glm::vec3(old_point, 0.f));
    }

#ifdef DEBUG
    if (mouse_ctx.is_selected()) {
        std::cout << "selected: " << mouse_ctx.get_selected() << std::endl;
    }
#endif

    env.camera.buffer(programs->get_selected_program());
}

void Context::init_mesh_prototypes(std::vector<Mesh> meshes) {
    mesh_factory.push(programs->get_selected_program(), meshes);
}
void Context::push_mesh_entity(std::vector<int> ids) {
    for (const auto& id : ids) {
        mesh_list.push_back(mesh_factory.get_mesh_entity(id));
    }
}

void Context::update_draw(MeshEntity& mesh_entity) {
    programs->bind(mesh_entity.get_shader());
    env.camera.buffer(programs->get_selected_program());
    if (mesh_entity.get_draw_mode() != DrawMode::WIREFRAME_ONLY) {
        draw_surfaces(mesh_entity);
    }
    if (mesh_entity.get_draw_mode() == DrawMode::WIREFRAME || mesh_entity.get_draw_mode() == DrawMode::WIREFRAME_ONLY) {
        draw_wireframes(mesh_entity);
    }
    else if (mesh_entity.get_draw_mode() == DrawMode::DRAW_NORMALS) {
        draw_normals(mesh_entity);
    }
}
void Context::update_draw(MeshEntity& mesh_entity, MeshEntityList& mesh_entities) {
    if (mesh_entity.get_dyn_reflections()) {
        env.draw_dynamic(*programs.get(), mesh_entity, mesh_entities, [&] (MeshEntity& sec_mesh) { update_draw(sec_mesh); });
    } else {
        env.cube_map_.bind();
    }
    update_draw(mesh_entity);
}
void Context::update_draw() {
    programs->reload();
    env.bind();
    update();
    for (MeshEntity& mesh_entity : mesh_list) {
        update_draw(mesh_entity, mesh_list);
    }
    env.draw(*programs.get());
}
void Context::draw_surfaces(MeshEntity& mesh_entity) {
    programs->bind(mesh_entity.get_shader());
    mesh_entity.draw(programs->get_selected_program());
}
void Context::draw_surfaces() {
    for (MeshEntity& mesh : mesh_list) {
        draw_surfaces(mesh);
    }
}
void Context::draw_wireframes(MeshEntity& mesh_entity) {
    ShaderPrograms selected = mesh_entity.get_shader();

    programs->bind(ShaderPrograms::DEF_SHADER);

    float min_zoom = 1.f / std::pow(2, 10);  // to prevent z-fighting

    auto temp = env.camera->get_view();
    // minimally scale the view to draw on top
    env.camera->scale_view(Camera::ScaleDir::Out, min_zoom);
    env.camera.buffer(programs->get_selected_program());
    mesh_entity.draw_wireframe(programs->get_selected_program());
    env.camera->set_view(temp);

    programs->bind(selected);
}
void Context::draw_wireframes() {
    for (MeshEntity& mesh : mesh_list) {
        draw_wireframes(mesh);
    }
}
void Context::draw_normals(MeshEntity& mesh_entity) {
    ShaderPrograms selected = mesh_entity.get_shader();

    programs->bind(ShaderPrograms::NORMALS);;
    env.camera.buffer(programs->get_selected_program());

    for (auto& mesh : mesh_list) {
        auto temp = mesh.get_color();
        mesh.set_color(glm::vec3(1.0, 0.0, 0.0));
        mesh.draw(programs->get_selected_program());
        mesh.set_color(temp);
    }

    mesh_entity.draw(programs->get_selected_program());

    programs->bind(selected);
}
void Context::draw_normals() {
    for (MeshEntity& mesh : mesh_list) {
        draw_normals(mesh);
    }
}