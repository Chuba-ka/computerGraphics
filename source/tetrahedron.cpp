#include "tetrahedron.hpp"

#include <algorithm>
#include <cmath>
#include <set>

namespace geometry {

float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

Vec3 normalized(Vec3 v) { return v * (1.0f / std::sqrt(dot(v, v))); }

Mesh truncatedTetrahedron() {
    // These four equidistant points form a regular tetrahedron centered at zero.
    const std::array<Vec3, 4> original = {{
        {1, 1, 1}, {1, -1, -1}, {-1, 1, -1}, {-1, -1, 1}
    }};
    Mesh mesh;
    int vertex_index[4][4]{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i == j) continue;
            vertex_index[i][j] = static_cast<int>(mesh.vertices.size());
            // Cut each edge at 1/3 and 2/3: all resulting sides have equal length.
            mesh.vertices.push_back((original[i] * 2.0f + original[j]) * (1.0f / 3.0f));
        }
    }

    auto add_face = [&](std::vector<int> indices, Vec3 normal) {
        normal = normalized(normal);
        Vec3 center{};
        for (int index : indices) center = center + mesh.vertices[index];
        center = center * (1.0f / static_cast<float>(indices.size()));
        const Vec3 u = normalized(mesh.vertices[indices.front()] - center);
        const Vec3 v = cross(normal, u);
        std::sort(indices.begin(), indices.end(), [&](int a, int b) {
            const Vec3 pa = mesh.vertices[a] - center;
            const Vec3 pb = mesh.vertices[b] - center;
            return std::atan2(dot(pa, v), dot(pa, u)) < std::atan2(dot(pb, v), dot(pb, u));
        });
        mesh.faces.push_back({std::move(indices), normal});
    };

    for (int i = 0; i < 4; ++i) {
        std::vector<int> triangle;
        for (int j = 0; j < 4; ++j) {
            if (i != j) triangle.push_back(vertex_index[i][j]);
        }
        add_face(std::move(triangle), original[i]);
    }
    for (int opposite = 0; opposite < 4; ++opposite) {
        std::vector<int> hexagon;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (i != j && i != opposite && j != opposite)
                    hexagon.push_back(vertex_index[i][j]);
            }
        }
        add_face(std::move(hexagon), original[opposite] * -1.0f);
    }

    std::set<std::array<int, 2>> edges;
    for (const auto& face : mesh.faces) {
        for (std::size_t i = 0; i < face.vertices.size(); ++i) {
            const int a = face.vertices[i];
            const int b = face.vertices[(i + 1) % face.vertices.size()];
            edges.insert({std::min(a, b), std::max(a, b)});
        }
    }
    mesh.edges.assign(edges.begin(), edges.end());
    return mesh;
}

Vec3 rotate(Vec3 p, Vec3 angles) {
    // Apply rotations about X, then Y, then Z (angles are in radians).
    const float cx = std::cos(angles.x), sx = std::sin(angles.x);
    const float cy = std::cos(angles.y), sy = std::sin(angles.y);
    const float cz = std::cos(angles.z), sz = std::sin(angles.z);
    p = {p.x, cx * p.y - sx * p.z, sx * p.y + cx * p.z};
    p = {cy * p.x + sy * p.z, p.y, -sy * p.x + cy * p.z};
    return {cz * p.x - sz * p.y, sz * p.x + cz * p.y, p.z};
}

} // namespace geometry
