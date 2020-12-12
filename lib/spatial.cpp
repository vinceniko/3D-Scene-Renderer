#include "spatial.h"

void Spatial::translate(glm::mat4 view_trans, glm::vec3 offset) {
    offset = glm::inverse(trans_) * glm::inverse(view_trans) * glm::vec4(offset, 0.0);
    trans_ = glm::translate(trans_, offset);;
}
void Spatial::scale(glm::mat4 view_trans, ScaleDir dir, float offset) {
    glm::mat4 trans = glm::translate(glm::mat4{ 1.f }, get_position());
    trans = glm::scale(trans, glm::vec3(dir == In ? 1 + offset : 1 - offset));
    trans = glm::translate(trans, -get_position());
    trans_ = trans_ * trans;
}
void Spatial::rotate(glm::mat4 view_trans, float degrees, glm::vec3 axis) {
    glm::vec3 view_axis = glm::vec3{ glm::inverse(trans_) * glm::inverse(view_trans) * glm::vec4{axis, 0.0} };

    glm::mat4 trans = glm::translate(glm::mat4{ 1.f }, get_position());
    trans = glm::rotate(trans, glm::radians(degrees), view_axis);
    trans = glm::translate(trans, -get_position());
    trans_ = trans_ * trans;
}
glm::vec3 Spatial::look_direction() {
    return trans_ * glm::vec4(0.f, 0.f, 1.f, 0.f);
}
glm::vec3 Spatial::get_position() {
    return trans_[3];
}

glm::mat4 Spatial::get_trans() {
    return trans_;
}