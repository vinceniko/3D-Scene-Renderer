#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include "helpers.h"

#include "definitions.h"
#include "transform.h"
#include "program.h"

class Camera {
    float fov_;
    float aspect_;

public:
    enum Projection { Ortho, Perspective };
    enum ZoomDir { Out, In };

protected:
    glm::mat4 view_trans_{1.f};
    Projection projection_mode_;

    float zoom_ = 1.f;

public:
    // ortho projection
    Camera();
    // perspective projection
    Camera(float aspect, float fov = 50.f);

    virtual void translate(glm::vec2 offset);
    virtual void translate(glm::vec2 new_point, glm::vec2 old_point);

    virtual void zoom(ZoomDir zoom_dir, float percent = 0.2);

    void switch_projection();
    void set_projection_mode(Projection projection);

    void set_aspect(float aspect) {
        aspect_ = aspect;
    } 

    glm::mat4 get_projection() const {
        return projection_mode_ == Projection::Perspective ? glm::perspective(glm::radians(fov_), aspect_, 0.1f, 100.f) : glm::ortho(-1.f, 1.f, -1.f, 1.f);
    }
    glm::mat4 get_view() const {
        return view_trans_;
    }
    glm::vec3 get_position() const {
        return glm::vec3(glm::inverse(view_trans_)[3]);
    }
};

class TrackballCamera : public Camera {
    float radius_ = 3.1f;
    float theta_ = glm::half_pi<float>();
    float phi_ = glm::half_pi<float>();

    float up_ = 1.0;
    
public:
    TrackballCamera();
    TrackballCamera(float aspect, float fov = 50.f);

    void zoom(ZoomDir zoom_dir, float percent = 0.2) override;
    void translate(glm::vec2 offset) override;
    void translate(glm::vec2 new_point, glm::vec2 old_point) override;
    void swivel();
};

class GLCamera : public TrackballCamera {
    uint id_;

    ProgramCtx& programs_;

    GLTransform view_uniform_;
    GLTransform projection_uniform_;

    void buffer_view_uniform();
    void buffer_projection_uniform();
public:
    GLCamera(ProgramCtx& programs);
    GLCamera(ProgramCtx& programs, float aspect, float fov = 50.f);
    
    void buffer();
};