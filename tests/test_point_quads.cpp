// Polygon mode POINT: a square around each vertex in front of the camera, of the same size at any distance.

#include <vector>

#include "pc/point_quads.h"
#include "test.h"

using namespace xport::pc;

namespace {
struct Vertex {
    float position[4];
    float colour;
};
}  // namespace

int main() {
    // At pixel (100, 50), w 2: positions are pixels times w.
    const std::vector<Vertex> points = {{{200, 100, 1, 2}, 0.5f}, {{10, 10, 1, -3}, 0.7f}, {{10, 10, 1, 0}, 0.9f}};
    std::vector<Vertex> out;
    append_point_quads(points, 2.0f, out);
    CHECK(out.size() == 6);  // the ones behind the camera are gone
    float left = 1e9f, right = -1e9f, top = 1e9f, bottom = -1e9f;
    for (const Vertex& v : out) {
        const float x = v.position[0] / v.position[3], y = v.position[1] / v.position[3];
        left = x < left ? x : left, right = x > right ? x : right;
        top = y < top ? y : top, bottom = y > bottom ? y : bottom;
        CHECK(v.position[3] == 2 && v.colour == 0.5f);
    }
    CHECK(left == 99 && right == 101 && top == 49 && bottom == 51);  // 2 pixels across, centred
    return test_result();
}
