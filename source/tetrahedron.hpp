#pragma once

#include <array>
#include <vector>

namespace geometry {

struct Vec3 {
    float x, y, z;
    Vec3 operator+(Vec3 v) const { return {x + v.x, y + v.y, z + v.z}; }
    Vec3 operator-(Vec3 v) const { return {x - v.x, y - v.y, z - v.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
};

float dot(Vec3 a, Vec3 b);
Vec3 cross(Vec3 a, Vec3 b);
Vec3 normalized(Vec3 v);

struct Face {
    // Counterclockwise when viewed from outside the solid.
    std::vector<int> vertices;
    Vec3 normal;
};

struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<Face> faces;
    std::vector<std::array<int, 2>> edges;
};

Mesh truncatedTetrahedron();
Vec3 rotate(Vec3 point, Vec3 angles);

} // namespace geometry
