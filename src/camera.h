#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <exception>

#include "helpers.h"

#include "definitions.h"
#include "transform.h"

class Camera {
    float fov_;
    float aspect_;

public:
    enum Projection { Ortho, Perspective };
    enum ZoomDir { Out, In };

    // ortho projection
    Camera();
    // perspective projection
    Camera(float aspect, float fov = 50.f);

    void translate(glm::vec2 offset);

    void zoom(ZoomDir zoom_dir, float percent = 0.2);

    void switch_projection();
    void perspective();
    void ortho();

    glm::mat4 get_projection() const {
        return projection_trans_;
    }
    glm::mat4 get_view() const {
        return view_trans_;
    }
    glm::vec3 get_position() const {
        return glm::vec3(glm::inverse(view_trans_)[3]);
    }

protected:
    glm::mat4 view_trans_{1.f};
    glm::mat4 projection_trans_;
    Projection projection_mode_;

    float zoom_ = 1.f;
};

class TrackballCamera : public Camera {
    float radius_ = 3.1f;
    float theta_ = glm::half_pi<float>();
    float phi_ = glm::half_pi<float>();

    float up_ = 1.0;
    
public:
    TrackballCamera();
    TrackballCamera(float aspect, float fov = 50.f);

    void zoom(ZoomDir zoom_dir, float percent = 0.2);
    void translate(glm::vec2 offset);
    void swivel();
};

class GLCamera : public TrackballCamera {
    uint id_;

    Program program_;

    GLTransform view_uniform_;
    GLTransform projection_uniform_;

    void update_view_uniform();
    void update_projection_uniform();
public:
    // TODO: pass in aspect
    GLCamera(const Program& program);

    GLCamera(const Program& program, float aspect, float fov = 50.f);
    
    void translate(glm::vec2 offset);
    void zoom(ZoomDir zoom_dir, float percent = 0.2);
    void swivel();
    void switch_projection();
};