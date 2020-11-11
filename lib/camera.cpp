#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

#ifdef DEBUG
#include <iostream>
#endif

#include "camera.h"

Camera::Camera() : projection_mode_(Ortho) {
    view_trans_ = glm::translate(view_trans_, glm::vec3(0.0f, 0.f, -2.f));
}
Camera::Camera(float aspect, float fov) : projection_mode_(Perspective), aspect_(aspect), fov_(fov) {
    view_trans_ = glm::translate(view_trans_, glm::vec3(0.0f, 0.0f, -2.f));
}

void Camera::translate(glm::vec2 offset) {
    view_trans_ = glm::translate(view_trans_, glm::vec3(offset, 0.f));
}
void Camera::translate(glm::vec2 new_point, glm::vec2 old_point) {
    view_trans_ = glm::translate(view_trans_, glm::vec3(new_point - old_point, 0.f));
}
void Camera::zoom(ZoomDir zoom_dir, float percent) {
    if (projection_mode_ == Projection::Ortho) return;

    glm::mat4 clone = glm::translate(glm::mat4(1.0f), glm::vec3(glm::inverse(view_trans_) * glm::vec4(glm::vec3(0.0), 1.0)));
    float zoom_perc = static_cast<bool>(zoom_dir) ? 1.f + percent : 1.f - percent;
    clone = glm::scale(clone, glm::vec3(zoom_perc, zoom_perc, 1.0));
    clone = glm::translate(clone, glm::vec3(-glm::inverse(view_trans_) * glm::vec4(glm::vec3(0.0), 1.0)));
    view_trans_ = view_trans_ * clone;
}

void Camera::switch_projection() {
    set_projection_mode(projection_mode_ == Projection::Perspective ? Projection::Ortho : Projection::Perspective);
}
void Camera::set_projection_mode(Camera::Projection projection) {
    projection_mode_ = projection;
}

void Camera::set_aspect(float aspect) {
    aspect_ = aspect;
} 

glm::mat4 Camera::get_projection() const {
    return projection_mode_ == Projection::Perspective ? glm::perspective(glm::radians(fov_), aspect_, 0.1f, 100.f) : glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -100.f, 100.f);
}
glm::mat4 Camera::get_view() const {
    return view_trans_;
}
glm::vec3 Camera::get_position() const {
    return glm::vec3(glm::inverse(view_trans_)[3]);
}

TrackballCamera::TrackballCamera() : Camera() {
    translate(glm::vec2(0.0));
}
TrackballCamera::TrackballCamera(float aspect, float fov) : Camera(aspect, fov) {
    translate(glm::vec2(0.0));
}

void TrackballCamera::zoom(ZoomDir zoom_dir, float percent) {
    // TODO: limit zoom in
    
    radius_ *= zoom_dir == Out ? 1.f + percent : 1.f - percent;
    translate(glm::vec2(0.0));
}
void TrackballCamera::translate(glm::vec2 offset) {
    if (up_ > 0.0f) {
        offset.x = -offset.x;
    }
    theta_ -= offset.x;

    phi_ -= offset.y;
    // Keep phi within -2PI to +2PI for easy 'up' comparison
    if (phi_ > glm::two_pi<float>()) {
        phi_ -= glm::two_pi<float>();
    } else if (phi_ < -glm::two_pi<float>()) {
        phi_ += glm::two_pi<float>();
    }

    // If phi is between 0 to PI or -PI to -2PI, make 'up' be positive Y, other wise make it negative Y
    if ((phi_ > 0 && phi_ < glm::pi<float>()) || (phi_ < -glm::pi<float>() && phi_ > -glm::two_pi<float>())) {
        up_ = 1.0f;
    } else {
        up_ = -1.0f;
    }

    // #ifdef DEBUG
    // std::cout << theta_ << std::endl;
    // std::cout << phi_ << std::endl;
    // #endif

    float camX =  radius_ * glm::sin(phi_) * glm::cos(theta_);
    float camY =  radius_ * glm::cos(phi_ );
    float camZ =  radius_ * glm::sin(phi_) * glm::sin(theta_);
    view_trans_ = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, up_, 0.0));
}
void TrackballCamera::translate(glm::vec2 new_point, glm::vec2 old_point) {
    const float diff_min = 0.001;  // hacky bug fix: drifting due to the camera
    auto diff = glm::vec2(new_point.x - old_point.x, -(new_point.y - old_point.y));
    translate(glm::abs(diff.x) > diff_min || glm::abs(diff.y) > diff_min ? diff : glm::vec2(0.f));
}
void TrackballCamera::swivel() {
    float camY =  radius_ * sin(theta_ + glfwGetTime()) * cos(phi_);
    float camZ =  radius_ * sin(theta_ + glfwGetTime()) * sin(phi_);
    float camX =  radius_ * cos(theta_ + glfwGetTime());
    view_trans_ = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
}

GLCamera::GLCamera(std::shared_ptr<ShaderProgramCtx> programs, std::shared_ptr<Camera> camera) :
    camera_(camera),
    programs_(programs),
    view_uniform_(programs, "view_trans", camera->get_view()), 
    projection_uniform_(programs, "projection", camera_->get_projection()) {}

void GLCamera::buffer_view_uniform() {
    view_uniform_.buffer(camera_->get_view());
}
void GLCamera::buffer_projection_uniform() {
    projection_uniform_.buffer(camera_->get_projection());
}
void GLCamera::buffer() {
    buffer_view_uniform();
    buffer_projection_uniform();
}

Camera& GLCamera::get_camera() {
    return *camera_.get();
}
void GLCamera::set_camera(std::shared_ptr<Camera> camera) {
    camera_ = camera;
}
Camera* GLCamera::operator ->() {
    return camera_.get();
}
const Camera* GLCamera::operator ->() const {
    return camera_.get();
}