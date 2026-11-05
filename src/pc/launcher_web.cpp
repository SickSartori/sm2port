#include "pc/launcher_web.h"

#include <algorithm>
#include <cmath>

namespace sm2::pc {
namespace {

constexpr float kPi = 3.14159265f;

float clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }
ImVec2 lerp(ImVec2 a, ImVec2 b, float t) { return ImVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
ImU32 white(float alpha) { return IM_COL32(236, 241, 255, static_cast<int>(255 * clamp01(alpha))); }

// An orb web centred at `c`, spokes spread over the angles a0..a1 (a full circle when a1 = a0 + 2 pi). `progress` 0..1
// weaves it: the spokes grow first, then the rings, each one as a thread running from spoke to spoke.
void orb(ImDrawList* list, ImVec2 c, float radius, float a0, float a1, int spokes, int rings, float progress, float now,
         float alpha, float thickness) {
    const float spoke_length = radius * clamp01(progress / 0.4f);
    std::vector<ImVec2> direction(spokes);
    for (int i = 0; i < spokes; ++i) {
        const float angle = a0 + (a1 - a0) * i / (spokes - 1) + 0.012f * std::sin(now * 0.5f + i);  // sways
        direction[i] = ImVec2(std::cos(angle), std::sin(angle));
        list->AddLine(c, ImVec2(c.x + direction[i].x * spoke_length, c.y + direction[i].y * spoke_length), white(alpha * 0.9f), thickness);
    }
    for (int ring = 1; ring <= rings; ++ring) {
        const float done = clamp01((progress - 0.35f - 0.65f * (ring - 1) / rings) / (0.65f / rings));
        if (done <= 0) break;
        const float r = radius * std::pow(static_cast<float>(ring) / rings, 1.15f) * (1 + 0.004f * std::sin(now * 0.7f + ring));
        for (int i = 0; i + 1 < spokes; ++i) {
            const float part = clamp01(done * (spokes - 1) - i);
            if (part <= 0) break;
            const ImVec2 p0(c.x + direction[i].x * r, c.y + direction[i].y * r);
            const ImVec2 p1(c.x + direction[i + 1].x * r, c.y + direction[i + 1].y * r);
            const float glint = 0.8f + 0.2f * std::sin(now * 1.4f - ring * 0.7f + i * 0.3f);
            if (part < 1) {
                list->AddLine(p0, lerp(p0, p1, part), white(alpha * glint), thickness);
            } else {  // a thread sags towards the centre
                const ImVec2 mid = lerp(p0, p1, 0.5f);
                list->AddBezierQuadratic(p0, ImVec2(c.x + (mid.x - c.x) * 0.86f, c.y + (mid.y - c.y) * 0.86f), p1,
                                         white(alpha * glint), thickness);
                if ((i + ring) % 4 == 0) {  // a drop of dew
                    list->AddCircleFilled(p1, thickness * 1.8f, white(alpha * (0.5f + 0.5f * std::sin(now * 2 + ring * 1.7f + i))));
                }
            }
        }
    }
}

}  // namespace

void WebEffects::burst(ImVec2 at, float radius, float now) { impacts_.push_back({at, now, radius}); }

void WebEffects::draw(ImDrawList* list, ImVec2 screen, float scale, float now, float dt, float since_open) {
    (void)dt;
    const float visible = clamp01(since_open / 1.0f);
    const float thick = std::max(1.0f, scale);
    orb(list, ImVec2(0, screen.y), screen.y * 0.62f, -kPi / 2, 0, 8, 10, clamp01(since_open / 3.4f), now, 0.34f * visible, thick);
    orb(list, ImVec2(screen.x, 0), screen.y * 0.46f, kPi / 2, kPi, 7, 8, clamp01((since_open - 0.7f) / 3.0f), now,
        0.30f * visible, thick);
    shots(list, screen, scale, now);
    impacts(list, scale, now);
    spider(list, screen, scale, now);
}

// Threads shot from outside the screen; each leaves a small web where it lands and then fades.
void WebEffects::shots(ImDrawList* list, ImVec2 screen, float scale, float now) {
    if (now >= next_shot_) {
        const int side = static_cast<int>(uniform(0, 2.999f));
        const ImVec2 from = side == 0 ? ImVec2(-20, uniform(0, screen.y * 0.8f))
                            : side == 1 ? ImVec2(uniform(0, screen.x), -20)
                                        : ImVec2(screen.x + 20, uniform(0, screen.y * 0.5f));
        shots_.push_back({from, ImVec2(uniform(0.12f, 0.88f) * screen.x, uniform(0.2f, 0.8f) * screen.y), now});
        next_shot_ = now + uniform(4, 8);
    }
    constexpr float kTravel = 0.28f, kHold = 2.2f, kFade = 1.2f;
    for (Shot& shot : shots_) {
        const float age = now - shot.start;
        const float t = clamp01(age / kTravel);
        const ImVec2 head = lerp(shot.from, shot.to, 1 - (1 - t) * (1 - t));
        const float alpha = age < kTravel + kHold ? 1.0f : 1 - clamp01((age - kTravel - kHold) / kFade);
        const float sag = scale * 14 * clamp01((age - kTravel) / 0.6f);
        const ImVec2 mid = lerp(shot.from, head, 0.5f);
        list->AddBezierQuadratic(shot.from, ImVec2(mid.x, mid.y + sag), head, white(0.55f * alpha), std::max(1.0f, scale * 1.3f));
        if (t < 1) {  // the head of the thread
            list->AddCircleFilled(head, scale * 7, white(0.12f));
            list->AddCircleFilled(head, scale * 3, white(0.9f));
        } else if (!shot.landed) {
            shot.landed = true;
            burst(shot.to, scale * uniform(55, 95), now);
        }
    }
    shots_.erase(std::remove_if(shots_.begin(), shots_.end(),
                                [&](const Shot& s) { return now - s.start > kTravel + kHold + kFade; }),
                 shots_.end());
}

void WebEffects::impacts(ImDrawList* list, float scale, float now) {
    for (const Impact& impact : impacts_) {
        const float age = now - impact.start;
        const float alpha = age < 1.6f ? 1.0f : 1 - clamp01((age - 1.6f) / 1.2f);
        orb(list, impact.at, impact.radius, 0, 2 * kPi, 9, 3, clamp01(age / 0.5f), now, 0.55f * alpha, std::max(1.0f, scale));
    }
    impacts_.erase(std::remove_if(impacts_.begin(), impacts_.end(), [&](const Impact& i) { return now - i.start > 2.8f; }),
                   impacts_.end());
}

// A small spider lowering itself on a thread from the top edge, waiting, and climbing back up.
void WebEffects::spider(ImDrawList* list, ImVec2 screen, float scale, float now) {
    constexpr float kCycle = 16;
    const float phase = std::fmod(now, kCycle) / kCycle;
    const float top = screen.y * 0.02f, bottom = screen.y * 0.2f;
    float y;
    if (phase < 0.4f) {
        const float t = phase / 0.4f;
        y = top + (bottom - top) * (t * t * (3 - 2 * t));
    } else if (phase < 0.6f) {
        y = bottom + scale * 4 * std::sin((phase - 0.4f) * 40);  // bobbing
    } else {
        const float t = clamp01((phase - 0.6f) / 0.3f);
        y = bottom + (top - bottom) * (t * t * (3 - 2 * t));
        if (phase > 0.9f) y = top - (phase - 0.9f) * screen.y * 0.6f;  // gone up out of sight
    }
    const float x = screen.x * 0.34f + std::sin(now * 0.5f) * scale * 14;
    list->AddLine(ImVec2(x, 0), ImVec2(x, y), white(0.4f), std::max(1.0f, scale));
    const float s = scale * 8;
    const ImU32 leg = IM_COL32(88, 90, 108, 255), body = IM_COL32(26, 26, 34, 255);
    for (int side = -1; side <= 1; side += 2) {
        for (int j = 0; j < 4; ++j) {
            const float wiggle = std::sin(now * 5 + j * 1.3f + side) * s * 0.25f;
            const ImVec2 hip(x + side * s * 0.35f, y + s * (0.1f + 0.28f * j));
            const ImVec2 knee(hip.x + side * s * (1.5f + 0.25f * j), hip.y - s * (1.0f - 0.55f * j) + wiggle);
            const ImVec2 foot(knee.x + side * s * 0.9f, knee.y + s * (1.7f + 0.1f * j));
            list->AddLine(hip, knee, leg, std::max(1.2f, scale * 1.4f));
            list->AddLine(knee, foot, leg, std::max(1.2f, scale * 1.4f));
        }
    }
    list->AddCircleFilled(ImVec2(x, y + s * 1.5f), s * 1.05f, body);
    list->AddCircleFilled(ImVec2(x, y), s * 0.62f, body);
    list->AddCircleFilled(ImVec2(x, y + s * 1.6f), s * 0.28f, IM_COL32(210, 36, 40, 255));  // the red mark
}

}  // namespace sm2::pc
