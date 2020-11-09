#include "triangle.h"

float area(Triangle tri) {
    return 0.5 * glm::length(glm::cross(tri[1] - tri[0], tri[2] - tri[1]));
}

glm::vec3 centroid(Triangle tri) {
    return 1 / 3.f * (tri[0] + tri[1] + tri[2]);
}