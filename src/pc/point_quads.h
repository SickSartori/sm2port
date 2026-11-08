#pragma once

#include <vector>

namespace sm2::pc {

// Polygon mode POINT (the night sky's stars): each vertex is a square of `size` surface pixels. Direct3D 11 only has
// points of one window pixel, which would shrink as the internal resolution grows. Positions are pixels times w, so the
// offset is too; a vertex behind the camera shows nothing.
template <class Vertex>
void append_point_quads(const std::vector<Vertex>& points, float size, std::vector<Vertex>& out) {
    out.reserve(out.size() + points.size() * 6);
    const float half = size / 2;
    for (const Vertex& point : points) {
        if (point.position[3] <= 0) continue;
        const auto corner = [&](float x, float y) {
            Vertex v = point;
            v.position[0] += x * half * point.position[3];
            v.position[1] += y * half * point.position[3];
            out.push_back(v);
        };
        corner(-1, -1), corner(1, -1), corner(1, 1);
        corner(-1, -1), corner(1, 1), corner(-1, 1);
    }
}

}  // namespace sm2::pc
