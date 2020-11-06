#pragma once

#include <glm/vec3.hpp>
#include <glm/ext/quaternion_geometric.hpp> // length, cross
#include <array>

constexpr int TRI = 3;
using Triangle = std::array<glm::vec3, TRI>;

float area(Triangle tri);

glm::vec3 centroid(Triangle tri);