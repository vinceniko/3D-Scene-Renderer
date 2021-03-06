#include "camera.h"

Camera::Camera(float aspect) : projection_mode_(Perspective), aspect_(aspect) {
    trans_ = glm::translate(trans_, glm::vec3(0.0f, 0.f, -2.f));
}
Camera::Camera(float aspect, float fov) : projection_mode_(Perspective), aspect_(aspect), fov_(fov) {
    trans_ = glm::translate(trans_, glm::vec3(0.0f, 0.0f, -2.f));
}

void Camera::switch_projection() {
    set_projection_mode(projection_mode_ == Projection::Perspective ? Projection::Ortho : Projection::Perspective);
}
void Camera::set_projection_mode(Camera::Projection projection) {
    projection_mode_ = projection;
}
void Camera::set_aspect(int width, int height) {
    set_aspect(static_cast<float>(width) / height);
}
void Camera::set_aspect(float aspect) {
    aspect_ = isnan(aspect) ? 1.f : aspect;
}
void Camera::set_fov(float fov) {
    fov_ = fov;
}
float Camera::get_aspect() {
    return aspect_;
}
float Camera::get_fov() {
    return fov_;
}
float Camera::get_up() {
    return up_;
}

glm::mat4 Camera::get_projection() const {
    // return projection_mode_ == Projection::Perspective ? glm::perspective(glm::radians(fov_), aspect_, 0.1f, 100.f) : glm::ortho((aspect_ <= 1 ? -aspect_ : -1.0f), (aspect_ <= 1 ? aspect_ : 1.0f), (aspect_ > 1 ? -1.f/aspect_ : -1.0f), (aspect_ > 1 ? 1.f/aspect_ : 1.0f), 0.1f, 100.f);
    return projection_mode_ == Projection::Perspective ? glm::perspective(glm::radians(fov_), aspect_, 0.001f, 100.f) : glm::ortho(-aspect_, aspect_, -1.f, 1.f, 0.001f, 100.f);
}
glm::mat4 Camera::get_view() const {
    return trans_;
}
glm::vec3 Camera::get_position() const {
    return glm::vec3(glm::inverse(trans_)[3]);
}
Camera::Projection Camera::get_projection_mode() const {
    return projection_mode_;
}
// gets the world ray direction of the nds coords
glm::vec3 Camera::get_ray_world(glm::vec2 nds_pos, float width, float height) const {
    glm::vec4 ray_clip = glm::vec4(nds_pos, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(get_projection()) * ray_clip;
    ray_eye = glm::vec4(glm::vec2(ray_eye), -1.0, 0.0);
    glm::vec3 ray_world = glm::inverse(get_view()) * ray_eye;
    ray_world = glm::normalize(ray_world);
    
    return ray_world;
}
// gets the world position of the nds coords
glm::vec3 Camera::get_pos_world(glm::vec2 nds_pos, float width, float height) const {
    glm::vec4 pos_clip = glm::vec4(nds_pos, -100.f /*far clipping plane: sufficiently negative*/, 1.0);
    glm::vec4 pos_eye = glm::inverse(get_projection()) * pos_clip;
    glm::vec3 pos_world = glm::inverse(get_view()) * pos_eye;
    
    return pos_world;
}

void Camera::set_view(glm::mat4 view) {
    trans_ = view;
}


void Camera::zoom_protected(ScaleDir zoom_dir, float percent) {
    // if (projection_mode_ == Projection::Ortho) return;

    zoom(zoom_dir, percent);
}

void Camera::scale_view(ScaleDir zoom_dir, float percent) {
    glm::vec3 view_origin = glm::inverse(trans_) * glm::vec4(glm::vec3(0.0, 0.0, 0.0), 1.0);

    glm::mat4 clone = glm::translate(glm::mat4(1.0f), view_origin);
    
    float zoom_perc = static_cast<bool>(zoom_dir) == ScaleDir::In ? 1.f + percent : 1.f - percent;
    
    clone = glm::scale(clone, glm::vec3(zoom_perc));
    clone = glm::translate(clone, -view_origin);
    
    trans_ *= clone;
}

void TwoDCamera::zoom(ScaleDir zoom_dir, float percent) {
    scale_view(zoom_dir, percent);
}

void TwoDCamera::translate(glm::vec3 offset) {
    trans_ = glm::translate(trans_, glm::vec3(offset) * intensity_scale_);
}
void TwoDCamera::translate(glm::vec3 new_point, glm::vec3 old_point) {
    translate(new_point - old_point);
}

void FreeCamera::translate(glm::vec3 offset) {
    TwoDCamera::translate(offset);
    trans_ = glm::lookAt(get_position(), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
}
void FreeCamera::translate(glm::vec3 new_point, glm::vec3 old_point) {
    translate(new_point - old_point);
}
void FreeCamera::zoom(ScaleDir zoom_dir, float percent) {
    float zoom_offset = static_cast<bool>(zoom_dir) ? percent : -percent;
    
    translate(glm::vec3(0.f, 0.f, zoom_offset));
}

TrackballCamera::TrackballCamera(float aspect) : Camera(aspect) {
    update_trans();
}
TrackballCamera::TrackballCamera(float aspect, float fov) : Camera(aspect, fov) {
    update_trans();
}

void TrackballCamera::update_trans() {
    float camX = radius_ * glm::sin(phi_) * glm::cos(theta_);
    float camY = radius_ * glm::cos(phi_);
    float camZ = radius_ * glm::sin(phi_) * glm::sin(theta_);
    trans_ = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0), glm::vec3(0.0, up_, 0.0));
    // scale to enable ortho zoom
    if (projection_mode_ == Camera::Projection::Ortho) {
        trans_ = glm::scale(trans_, ortho_scale_); // reinstate scale
    }
}

void TrackballCamera::zoom(ScaleDir zoom_dir, float percent) {
    // TODO: limit zoom in

    if (projection_mode_ == Projection::Perspective) {
        radius_ *= zoom_dir == In ? 1.f - percent : 1.f + percent;  // increase radius to zoom out
        update_trans();
    } else {
        scale_view(zoom_dir, percent);
    }
}

void TrackballCamera::switch_projection() {
    Camera::switch_projection();
    update_trans();    
}

void TrackballCamera::scale_view(ScaleDir zoom_dir, float percent) {
    Camera::scale_view(zoom_dir, percent);
    if (projection_mode_ == Camera::Projection::Ortho) {
        ortho_scale_ = glm::vec3(glm::length(trans_[0]), glm::length(trans_[1]), glm::length(trans_[2]));
    }
}

void TrackballCamera::translate(glm::vec3 offset) {
    if (up_ > 0.0f) {
        offset.x = -offset.x;
    }
    theta_ -= offset.x;

    phi_ -= offset.y;
    // Keep phi within -2PI to +2PI for easy 'up' comparison
    if (phi_ > glm::two_pi<float>()) {
        phi_ -= glm::two_pi<float>();
    }
    else if (phi_ < -glm::two_pi<float>()) {
        phi_ += glm::two_pi<float>();
    }

    // If phi is between 0 to PI or -PI to -2PI, make 'up' be positive Y, other wise make it negative Y
    if ((phi_ > 0 && phi_ < glm::pi<float>()) || (phi_ < -glm::pi<float>() && phi_ > -glm::two_pi<float>())) {
        up_ = 1.0f;
    }
    else {
        up_ = -1.0f;
    }

    // #ifdef DEBUG
    // std::cout << theta_ << std::endl;
    // std::cout << phi_ << std::endl;
    // #endif

    float camX = radius_ * glm::sin(phi_) * glm::cos(theta_);
    float camY = radius_ * glm::cos(phi_);
    float camZ = radius_ * glm::sin(phi_) * glm::sin(theta_);
    update_trans();
}
void TrackballCamera::translate(glm::vec3 new_point, glm::vec3 old_point) {
    auto diff = glm::vec3(new_point.x - old_point.x, -(new_point.y - old_point.y), 0.f) * 2.f;
    translate(diff);
}
void TrackballCamera::swivel() {
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time_);
    float dur_val = dur.count() / 1000.f;
    float camY = radius_ * sin(theta_ + dur_val) * cos(phi_);
    float camZ = radius_ * sin(theta_ + dur_val) * sin(phi_);
    float camX = radius_ * cos(theta_ + dur_val);
    trans_ = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
}

RenderCamera::RenderCamera(std::unique_ptr<Camera> camera) :
    camera_(std::move(camera)),
    view_uniform_("u_view_trans"),
    projection_uniform_("u_projection") {}

void RenderCamera::buffer_view_uniform() {
    view_uniform_.buffer(camera_->get_view());
}
void RenderCamera::buffer_projection_uniform() {
    projection_uniform_.buffer(camera_->get_projection());
}
void RenderCamera::buffer() {
    buffer_view_uniform();
    buffer_projection_uniform();
}

Camera& RenderCamera::get_camera() {
    return *camera_.get();
}
Camera* RenderCamera::get_camera_ptr() {
    return camera_.get();
}
std::unique_ptr<Camera> RenderCamera::get_camera_move() {
    return std::move(camera_);
}
void RenderCamera::set_camera(std::unique_ptr<Camera> camera) {
    camera_ = std::move(camera);
}
Camera* RenderCamera::operator ->() {
    return camera_.get();
}
const Camera* RenderCamera::operator ->() const {
    return camera_.get();
}