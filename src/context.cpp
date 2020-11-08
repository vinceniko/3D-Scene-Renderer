#include "context.h"

void MouseContext::hold() {
    held_ = true;
}
void MouseContext::hold(size_t i) {
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
void MouseContext::set_world_point(glm::vec2 world_point) {
    prev_world_point_ = world_point_;
    world_point_ = world_point;
}
glm::vec2 MouseContext::get_world_point() const {
    return world_point_;
}
glm::vec2 MouseContext::get_prev_world_point() const {
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

GLContext::GLContext(Program& program) : 
    program_(program), camera(program_), mesh_ctx(program_) {}
GLContext::GLContext(Program& program, GLCamera camera) : 
    program_(program), camera(camera), mesh_ctx(program_) {}

int GLContext::intersected_mesh(glm::vec3 world_ray_dir) const {
    float min_dist = std::numeric_limits<float>::infinity();
    int closest = -1;
    for (size_t i = 0; i < mesh_list.size(); i++) {
        float distance = mesh_list[i].intersected_triangles(camera.get_position(), world_ray_dir);
        if (distance >= 0) {
            if (min_dist > distance) {
                min_dist = distance;
                closest = i;
            }
        }
    }
    return closest;
}
void GLContext::select(glm::vec3 world_ray_dir) {
    int found = intersected_mesh(world_ray_dir);
    if (found >= 0 && found != mouse_ctx.get_selected()) {
        deselect();
        mouse_ctx.hold(found);
        mesh_list[found].set_color(glm::vec3(1.f) - mesh_list[found].get_color());
    } else if (found >= 0) {
        deselect();
    }
}
void GLContext::deselect() {
    if (mouse_ctx.is_selected()) {
        mesh_list[mouse_ctx.get_selected()].set_color(glm::vec3(1.f) - mesh_list[mouse_ctx.get_selected()].get_color());   
    }
    mouse_ctx.deselect();
}
Optional<MeshEntity> GLContext::get_selected() {
    if (mouse_ctx.is_selected()) {
        return {{mesh_list[mouse_ctx.get_selected()] }};
    }

    return {};
}

void GLContext::update() {
    glm::vec2 old_point = mouse_ctx.get_prev_world_point();
    glm::vec2 new_point = mouse_ctx.get_world_point();

    // #ifdef DEBUG
    // std::cout << "new_point: " << new_point[0] << ' ' << new_point[1] << ' ' << 
    // "old_point: " << old_point[0] << ' ' << old_point[1] << ' ' << std::endl;
    // #endif

    if (mouse_ctx.is_held()) {
        camera.translate(new_point, old_point);
    }

    #ifdef DEBUG
    if (mouse_ctx.is_selected()) {
        std::cout << "selected: " << mouse_ctx.get_selected() << std::endl;
    }
    #endif
    
    camera.update();
}