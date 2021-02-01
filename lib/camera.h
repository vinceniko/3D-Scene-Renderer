#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <memory>
#include <chrono>

#include "definitions.h"
#include "renderer.h"
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
    friend class RenderCamera;

    // defaults to perspective projection
    Camera(float aspect);
    Camera(float aspect, float fov);
    virtual ~Camera() = default;

    virtual void translate(glm::vec3 offset) = 0;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) = 0;

    virtual void scale_view(ScaleDir zoom_dir, float percent = 0.2);
    virtual void zoom(ScaleDir zoom_dir, float percent = 0.2) =0;
    // prevents zooming in orthographic mode
    void zoom_protected(ScaleDir zoom_dir, float percent = 0.2);

    virtual void switch_projection();
    virtual void set_projection_mode(Projection projection);

    virtual void set_aspect(float aspect);
    virtual void set_aspect(int width, int height);
    virtual void set_fov(float fov);
    virtual void set_position(glm::vec3 new_pos) {
        trans_[3] = glm::inverse(trans_) * glm::vec4(new_pos, 1.0);
    }

    virtual float get_aspect();
    virtual float get_fov();
    virtual float get_up();
    virtual glm::mat4 get_projection() const;
    virtual Projection get_projection_mode() const;
    virtual glm::mat4 get_view() const;
    virtual void set_view(glm::mat4 view);
    virtual glm::vec3 get_position() const;

    // gets the world ray direction of the nds coords
    virtual glm::vec3 get_ray_world(glm::vec2 nds_pos, float width, float height) const;

    // gets the world position of the nds coords
    virtual glm::vec3 get_pos_world(glm::vec2 nds_pos, float width, float height) const;
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

    // for ortho zoom
    glm::vec3 ortho_scale_ = glm::vec3(1.f);

    // for swivel
    std::chrono::steady_clock::time_point start_time_ = std::chrono::steady_clock::now();

public:
    TrackballCamera(float aspect);
    TrackballCamera(float aspect, float fov);

    virtual void zoom(ScaleDir zoom_dir, float percent = 0.2) override;
    virtual void translate(glm::vec3 offset) override;
    virtual void translate(glm::vec3 new_point, glm::vec3 old_point) override;
    virtual void swivel();

    virtual void switch_projection() override;
    // sets ortho scale
    virtual void scale_view(ScaleDir zoom_dir, float percent = 0.2) override;

    void update_trans();
};

// stores a Camera or descendent type and necessary gl info to bind to the appropriate uniforms such as the view and projection matrix transforms
class RenderCamera {
    std::unique_ptr<Camera> camera_;

    Uniform view_uniform_;
    Uniform projection_uniform_;

    void buffer_view_uniform();
    void buffer_projection_uniform();
public:
    RenderCamera(std::unique_ptr<Camera> camera);

    Camera& get_camera();
    Camera* get_camera_ptr();
    std::unique_ptr<Camera> get_camera_move();
    void set_camera(std::unique_ptr<Camera> camera);
    Camera* operator ->();
    const Camera* operator ->() const;

    // buffers the data. used to update gl state after mutating program state
    void buffer();
};