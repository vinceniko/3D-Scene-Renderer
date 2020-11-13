#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <memory>
#include <chrono>

#include "helpers.h"

#include "definitions.h"
#include "transform.h"
#include "shader.h"

#ifdef DEBUG
#include <iostream>
#endif

class Camera {

public:
    enum Projection { Ortho, Perspective };
    enum ZoomDir { Out, In };

protected:
    const float fov_init_ = 50.f;

    float fov_ = fov_init_;

    const float intensity_init_ = 2.f;
    float intensity_scale_ = intensity_init_;
    
    float aspect_;

    glm::mat4 view_trans_{ 1.f };
    Projection projection_mode_;

public:
    friend class GLCamera;

    // ortho projection
    Camera();
    // perspective projection
    Camera(float aspect, float fov = 50.f);

    virtual void translate(glm::vec3 offset) = 0;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) = 0;

    virtual void scale_view(ZoomDir zoom_dir, float percent = 0.2);
    virtual void zoom(ZoomDir zoom_dir, float percent = 0.2) =0;
    // prevents zooming in orthographic mode
    void zoom_protected(ZoomDir zoom_dir, float percent = 0.2);

    virtual void switch_projection();
    virtual void set_projection_mode(Projection projection);

    virtual void set_aspect(float aspect);

    virtual glm::mat4 get_projection() const;
    virtual Projection get_projection_mode() const;
    virtual glm::mat4 get_view() const;
    virtual void set_view(glm::mat4 view);
    virtual glm::vec3 get_position() const;

    // gets the world ray direction of the nds coords
    glm::vec3 get_ray_world(glm::vec2 nds_pos, float width, float height) const;

    // gets the world position of the nds coords
    glm::vec3 get_pos_world(glm::vec2 nds_pos, float width, float height) const;
};

class TwoDCamera : public Camera {
    using Camera::Camera;

    float zoom_ = 1.f;

public:
    virtual void zoom(ZoomDir zoom_dir, float percent = 0.2) override;
    virtual void translate(glm::vec3 offset) override;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) override;
};

class FreeCamera : public TwoDCamera {
protected:
    virtual void zoom(ZoomDir zoom_dir, float percent = 0.2) override;

public:    
    using TwoDCamera::TwoDCamera;

    virtual void translate(glm::vec3 offset) override;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) override;
};

class TrackballCamera : public Camera {
    float radius_ = 3.1f;
    float theta_ = glm::half_pi<float>();
    float phi_ = glm::half_pi<float>();

    float up_ = 1.0;

    // for swivel
    std::chrono::steady_clock::time_point start_time_ = std::chrono::steady_clock::now();

public:
    TrackballCamera();
    TrackballCamera(float aspect, float fov = 50.f);

    virtual void zoom(ZoomDir zoom_dir, float percent = 0.2) override;
    virtual void translate(glm::vec3 offset) override;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) override;
    virtual void swivel();
};

// stores a Camera or descendent type and necessary gl info to bind to the appropriate uniforms such as the view and projection matrix transforms
class GLCamera {
    std::shared_ptr<Camera> camera_;

    std::reference_wrapper<ShaderProgramCtx> programs_;

    GLTransform view_uniform_;
    GLTransform projection_uniform_;

    void buffer_view_uniform();
    void buffer_projection_uniform();
public:
    GLCamera(ShaderProgramCtx& programs, std::shared_ptr<Camera> camera);

    Camera& get_camera();
    std::shared_ptr<Camera> get_camera_ptr();
    void set_camera(std::shared_ptr<Camera> camera);
    void set_camera(std::shared_ptr<Camera>&& camera);
    Camera* operator ->();
    const Camera* operator ->() const;

    // buffers the data. used to update gl state after mutating program state
    void buffer();
};