#include "tetrahedron.hpp"

#include <cmath>
#include <iostream>
#include <map>
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    const auto mesh = geometry::truncatedTetrahedron();
    require(mesh.vertices.size() == 12, "Expected 12 vertices");
    require(mesh.edges.size() == 18, "Expected 18 edges");
    require(mesh.faces.size() == 8, "Expected 8 faces");
    std::map<std::array<int, 2>, int> edge_faces;
    std::array<int, 12> degree{};
    int triangles = 0, hexagons = 0;
    constexpr float tolerance = 1e-5f;
    const float expected_length = std::sqrt(8.0f) / 3.0f;
    for (const auto& face : mesh.faces) {
        const auto count = face.vertices.size();
        triangles += count == 3;
        hexagons += count == 6;
        require(count == 3 || count == 6, "Unexpected face type");
        geometry::Vec3 center{};
        for (int index : face.vertices) center = center + mesh.vertices[index];
        center = center * (1.0f / static_cast<float>(count));
        require(geometry::dot(face.normal, center) > 0, "Normal must point outward");
        const auto radius = mesh.vertices[face.vertices[0]] - center;
        for (std::size_t i = 0; i < count; ++i) {
            const int a = face.vertices[i], b = face.vertices[(i + 1) % count];
            const int c = face.vertices[(i + 2) % count];
            const auto p = mesh.vertices[a], q = mesh.vertices[b], r = mesh.vertices[c];
            const auto edge = q - p;
            const auto offset = p - center;
            require(std::abs(std::sqrt(geometry::dot(edge, edge)) - expected_length) < tolerance,
                    "All edges must have length sqrt(8)/3");
            require(std::abs(geometry::dot(offset, face.normal)) < tolerance, "Face must be planar");
            require(std::abs(geometry::dot(offset, offset) - geometry::dot(radius, radius)) < tolerance,
                    "Face vertices must lie on a common circle");
            require(geometry::dot(geometry::cross(edge, r - q), face.normal) > 0,
                    "Face must be convex with outward winding");
            const float cosine = geometry::dot(p - q, r - q) / (expected_length * expected_length);
            require(std::abs(cosine - (count == 3 ? 0.5f : -0.5f)) < tolerance,
                    "Face must have regular interior angles");
            ++edge_faces[{std::min(a, b), std::max(a, b)}];
        }
    }
    require(triangles == 4 && hexagons == 4, "Expected four faces of each type");
    for (const auto& edge : mesh.edges) {
        require(edge_faces[edge] == 2, "Every edge must belong to two faces");
        ++degree[edge[0]];
        ++degree[edge[1]];
    }
    for (int value : degree) require(value == 3, "Every vertex must have degree three");
    for (auto p : mesh.vertices) {
        const auto rotated = geometry::rotate(p, {0.7f, -1.2f, 2.1f});
        require(std::abs(geometry::dot(p, p) - geometry::dot(rotated, rotated)) < tolerance,
                "Rotation must preserve distance to origin");
    }
    std::cout << "Truncated tetrahedron geometry: all checks passed\n";
}
