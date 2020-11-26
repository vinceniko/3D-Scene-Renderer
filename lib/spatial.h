#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

class Spatial {
protected:
    glm::mat4 trans_{ 1.f };
public:
    enum ScaleDir {
        Out,
        In,
    };

    virtual void translate(glm::mat4 view_trans, glm::vec3 offset) {};
    virtual void scale(glm::mat4 view_trans, ScaleDir dir, float offset) {};
    virtual void rotate(glm::mat4 view_trans, float degrees, glm::vec3 axis) {};
};