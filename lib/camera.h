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
    const float fov_init = 50.f;
    
    float fov_ = fov_init;
    float aspect_;

public:
    enum Projection { Ortho, Perspective };
    enum ZoomDir { Out, In };

protected:
    glm::mat4 view_trans_{1.f};
    Projection projection_mode_;

    float zoom_ = 1.f;

public:
    friend class GLCamera;

    // ortho projection
    Camera();
    // perspective projection
    Camera(float aspect, float fov = 50.f);

    virtual void translate(glm::vec2 offset);
    virtual void translate(glm::vec2 new_point, glm::vec2 old_point);

    virtual void zoom(ZoomDir zoom_dir, float percent = 0.2);

    void switch_projection();
    void set_projection_mode(Projection projection);

    void set_aspect(float aspect);

    glm::mat4 get_projection() const;
    glm::mat4 get_view() const;
    glm::vec3 get_position() const;
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

    void zoom(ZoomDir zoom_dir, float percent = 0.2) override;
    void translate(glm::vec2 offset) override;
    void translate(glm::vec2 new_point, glm::vec2 old_point) override;
    void swivel();
};

// stores a Camera or descendent type and necessary gl info to bind to the appropriate uniforms such as the view and projection matrix transforms
class GLCamera {
    std::shared_ptr<Camera> camera_;

    std::shared_ptr<ShaderProgramCtx> programs_;

    GLTransform view_uniform_;
    GLTransform projection_uniform_;

    void buffer_view_uniform();
    void buffer_projection_uniform();
public:
    GLCamera(std::shared_ptr<ShaderProgramCtx> programs, std::shared_ptr<Camera> camera);

    Camera& get_camera();
    void set_camera(std::shared_ptr<Camera> camera);
    Camera* operator ->();
    const Camera* operator ->() const;
    
    // buffers the data. used to update gl state after mutating program state
    void buffer();
};