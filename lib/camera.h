#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <memory>
#include <chrono>

#include "definitions.h"
#include "transform.h"
#include "shader.h"
#include "spatial.h"

#ifdef DEBUG
#include <iostream>
#endif

class Camera : public Spatial {

public:
    enum Projection { Ortho, Perspective };

protected:
    float fov_ = 45.f;

    float intensity_scale_ = 2.f;
    
    float aspect_;

    float up_ = 1.0;

    Projection projection_mode_;

public:
    friend class GLCamera;

    // defaults to perspective projection
    Camera(float aspect);
    Camera(float aspect, float fov);

    virtual void translate(glm::vec3 offset) = 0;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) = 0;

    virtual void scale_view(ScaleDir zoom_dir, float percent = 0.2);
    virtual void zoom(ScaleDir zoom_dir, float percent = 0.2) =0;
    // prevents zooming in orthographic mode
    void zoom_protected(ScaleDir zoom_dir, float percent = 0.2);

    virtual void switch_projection();
    virtual void set_projection_mode(Projection projection);

    virtual void set_aspect(float aspect);
    virtual void set_fov(float fov);

    virtual float get_aspect_();
    virtual float get_fov();
    virtual float get_up();
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
    virtual void zoom(ScaleDir zoom_dir, float percent = 0.2) override;
    virtual void translate(glm::vec3 offset) override;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) override;
};

class FreeCamera : public TwoDCamera {
protected:
    virtual void zoom(ScaleDir zoom_dir, float percent = 0.2) override;

public:    
    using TwoDCamera::TwoDCamera;

    virtual void translate(glm::vec3 offset) override;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) override;
};

class TrackballCamera : public Camera {
    float radius_ = 3.1f;
    float theta_ = glm::half_pi<float>();
    float phi_ = glm::half_pi<float>();

    // for swivel
    std::chrono::steady_clock::time_point start_time_ = std::chrono::steady_clock::now();

public:
    TrackballCamera(float aspect);
    TrackballCamera(float aspect, float fov);

    virtual void zoom(ScaleDir zoom_dir, float percent = 0.2) override;
    virtual void translate(glm::vec3 offset) override;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) override;
    virtual void swivel();

    void update_trans();
};

// stores a Camera or descendent type and necessary gl info to bind to the appropriate uniforms such as the view and projection matrix transforms
class GLCamera {
    std::shared_ptr<Camera> camera_;

    GLTransform view_uniform_;
    GLTransform projection_uniform_;

    void buffer_view_uniform(ShaderProgram& program);
    void buffer_projection_uniform(ShaderProgram& program);
public:
    GLCamera(std::shared_ptr<Camera> camera);

    Camera& get_camera();
    std::shared_ptr<Camera> get_camera_ptr();
    void set_camera(std::shared_ptr<Camera> camera);
    void set_camera(std::shared_ptr<Camera>&& camera);
    Camera* operator ->();
    const Camera* operator ->() const;

    // buffers the data. used to update gl state after mutating program state
    void buffer(ShaderProgram& program);
};