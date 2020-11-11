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

Context::Context(std::unique_ptr<ShaderProgramCtx> programs) :
    programs(std::move(programs)), camera(this->programs, std::shared_ptr<Camera>{new Camera()}), mesh_factory(this->programs) {}

Context::Context(std::unique_ptr<ShaderProgramCtx> programs, std::shared_ptr<Camera> new_cam) :
    programs(std::move(programs)), camera(this->programs, new_cam), mesh_factory(this->programs) {}

int Context::intersected_mesh(glm::vec3 world_ray_dir) const {
    float min_dist = std::numeric_limits<float>::infinity();
    int closest = -1;
    for (size_t i = 0; i < mesh_list.size(); i++) {
        float distance = mesh_list[i].intersected_triangles(camera->get_position(), world_ray_dir);
        if (distance >= 0) {
            if (min_dist > distance) {
                min_dist = distance;
                closest = i;
            }
        }
    }
    return closest;
}
void Context::select(glm::vec3 world_ray_dir) {
    int found = intersected_mesh(world_ray_dir);
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
        camera->translate(new_point, old_point);
    }

#ifdef DEBUG
    if (mouse_ctx.is_selected()) {
        std::cout << "selected: " << mouse_ctx.get_selected() << std::endl;
    }
#endif

    camera.buffer();
}

void Context::init_mesh_prototypes(std::vector<Mesh> meshes) {
    mesh_factory.push(meshes);
}
void Context::push_mesh_entity(std::vector<size_t> ids) {
    for (const auto& id : ids) {
        mesh_list.push_back(mesh_factory.get_mesh_entity(id));
    }
}

void Context::switch_draw_mode() {
    draw_mode = static_cast<DrawMode>((static_cast<int>(draw_mode) + 1) % DrawMode::SIZE);
#ifdef DEBUG
    std::cout << "draw_mode: " << draw_mode << std::endl;
#endif
}

void Context::switch_program() {
    programs->bind(shaders[(shader_idx += 1) %= shaders.size()]);
}

void Context::draw() {
    programs->bind(programs->get_selected());
    draw_surface();
    if (draw_mode == DrawMode::WIREFRAME) {
        draw_wireframe();
    }
    else if (draw_mode == DrawMode::NORMALS) {
        draw_normals();
    }
}
void Context::draw_surface() {
    update();
    mesh_list.draw();
}
void Context::draw_wireframe() {
    float min_zoom = 1.f / 4096.f;  // to prevent z-fighting

    camera->zoom(Camera::ZoomDir::In, min_zoom);
    camera.buffer();
    mesh_list.draw_wireframe();
    camera->zoom(Camera::ZoomDir::In, -min_zoom);
}
void Context::draw_normals() {
    ShaderPrograms selected = programs->get_selected();

    programs->bind(ShaderPrograms::NORMALS);;
    camera.buffer();

    for (auto& mesh : mesh_list) {
        auto temp = mesh.get_color();
        mesh.set_color(glm::vec3(1.0, 0.0, 0.0));
        mesh.draw();
        mesh.set_color(temp);
    }

    mesh_list.draw();

    programs->bind(selected);
}