#pragma once

// Spider-web effects drawn behind the launcher: an orb web in two corners that weaves itself when the launcher opens
// and then sways and glints, web threads shot across the screen that leave a small web where they land, a spider
// going up and down its thread, and a small web for each click and page change.

#include <random>
#include <vector>

#include "imgui.h"

namespace xport::pc {

class WebEffects {
public:
    // `scale`: the launcher's size factor (1 at 1600x900); `since_open`: seconds since the launcher opened.
    void draw(ImDrawList* list, ImVec2 screen, float scale, float now, float dt, float since_open);
    void burst(ImVec2 at, float radius, float now);  // a small web landing at `at`

private:
    struct Shot {
        ImVec2 from, to;
        float start;
        bool landed = false;
    };
    struct Impact {
        ImVec2 at;
        float start, radius;
    };

    float uniform(float low, float high) { return std::uniform_real_distribution<float>(low, high)(random_); }
    void shots(ImDrawList* list, ImVec2 screen, float scale, float now);
    void impacts(ImDrawList* list, float scale, float now);
    void spider(ImDrawList* list, ImVec2 screen, float scale, float now);

    std::vector<Shot> shots_;
    std::vector<Impact> impacts_;
    float next_shot_ = 4;
    std::mt19937 random_{std::random_device{}()};
};

}  // namespace xport::pc
