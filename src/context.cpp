#include "context.h"

void MouseContext::hold() {
    selected_ = -1;
    held_ = true;
}
void MouseContext::hold(size_t i) {
    selected_ = i;
    held_ = true;
}
void MouseContext::release() {
    held_ = false;
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