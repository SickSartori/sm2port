#include "pc/launcher_web.h"

#include <algorithm>
#include <cmath>

namespace xport::pc {
namespace {

constexpr float kPi = 3.14159265f;

float clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }
ImVec2 lerp(ImVec2 a, ImVec2 b, float t) { return ImVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
ImU32 white(float alpha) { return IM_COL32(236, 241, 255, static_cast<int>(255 * clamp01(alpha))); }

// A point of a web pulled towards `pull`, the more the nearer it is (none beyond `reach`).
ImVec2 stretch(ImVec2 p, ImVec2 pull, float reach) {
    const float dx = pull.x - p.x, dy = pull.y - p.y, distance = std::sqrt(dx * dx + dy * dy);
    if (distance >= reach) return p;
    const float k = 0.22f * (1 - distance / reach) * (1 - distance / reach);
    return ImVec2(p.x + dx * k, p.y + dy * k);
}

// An orb web centred at `c`, spokes spread over the angles a0..a1 (a full circle when a1 = a0 + 2 pi). `progress` 0..1
// weaves it: the spokes grow first, then the rings, each one as a thread running from spoke to spoke.
void orb(ImDrawList* list, ImVec2 c, float radius, float a0, float a1, int spokes, int rings, float progress, float now,
         float alpha, float thickness, ImVec2 pull, float reach) {
    const float spoke_length = radius * clamp01(progress / 0.4f);
    std::vector<ImVec2> direction(spokes);
    for (int i = 0; i < spokes; ++i) {
        const float angle = a0 + (a1 - a0) * i / (spokes - 1) + 0.012f * std::sin(now * 0.5f + i);  // sways
        direction[i] = ImVec2(std::cos(angle), std::sin(angle));
        const ImVec2 end(c.x + direction[i].x * spoke_length, c.y + direction[i].y * spoke_length);
        list->AddBezierQuadratic(c, stretch(lerp(c, end, 0.6f), pull, reach), end, white(alpha * 0.9f), thickness);
    }
    for (int ring = 1; ring <= rings; ++ring) {
        const float done = clamp01((progress - 0.35f - 0.65f * (ring - 1) / rings) / (0.65f / rings));
        if (done <= 0) break;
        const float r = radius * std::pow(static_cast<float>(ring) / rings, 1.15f) * (1 + 0.004f * std::sin(now * 0.7f + ring));
        for (int i = 0; i + 1 < spokes; ++i) {
            const float part = clamp01(done * (spokes - 1) - i);
            if (part <= 0) break;
            const ImVec2 p0 = stretch(ImVec2(c.x + direction[i].x * r, c.y + direction[i].y * r), pull, reach);
            const ImVec2 p1 = stretch(ImVec2(c.x + direction[i + 1].x * r, c.y + direction[i + 1].y * r), pull, reach);
            const float glint = 0.8f + 0.2f * std::sin(now * 1.4f - ring * 0.7f + i * 0.3f);
            if (part < 1) {
                list->AddLine(p0, lerp(p0, p1, part), white(alpha * glint), thickness);
            } else {  // a thread sags towards the centre
                const ImVec2 mid = lerp(p0, p1, 0.5f);
                list->AddBezierQuadratic(p0, stretch(ImVec2(c.x + (mid.x - c.x) * 0.86f, c.y + (mid.y - c.y) * 0.86f), pull, reach), p1,
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

void WebEffects::draw(ImDrawList* list, ImVec2 screen, float scale, float now, float dt, float since_open, ImVec2 pointer,
                      ImVec2 spider_at) {
    const float visible = clamp01(since_open / 1.0f);
    const float thick = std::max(1.0f, scale);
    if (pointer.x < 0) pointer = ImVec2(-10000, -10000);
    // The pull springs after the pointer.
    pull_ = pull_.x < -9999 || pointer.x < -9999 ? pointer : lerp(pull_, pointer, std::min(1.0f, dt * 6));
    const float reach = scale * 260;
    orb(list, ImVec2(0, screen.y), screen.y * 0.62f, -kPi / 2, 0, 8, 10, clamp01(since_open / 3.4f), now, 0.34f * visible, thick,
        pull_, reach);
    orb(list, ImVec2(screen.x, 0), screen.y * 0.46f, kPi / 2, kPi, 7, 8, clamp01((since_open - 0.7f) / 3.0f), now,
        0.30f * visible, thick, pull_, reach);
    shots(list, screen, scale, now);
    impacts(list, scale, now);
    spider(list, screen, scale, now, dt, spider_at);
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
        orb(list, impact.at, impact.radius, 0, 2 * kPi, 9, 3, clamp01(age / 0.5f), now, 0.55f * alpha, std::max(1.0f, scale),
            pull_, scale * 260);
    }
    impacts_.erase(std::remove_if(impacts_.begin(), impacts_.end(), [&](const Impact& i) { return now - i.start > 2.8f; }),
                   impacts_.end());
}

// A small spider lowering itself on a thread from the top edge, waiting, and climbing back up.
void WebEffects::spider(ImDrawList* list, ImVec2 screen, float scale, float now, float dt, ImVec2 at) {
    if (at.y >= 0) {  // hangs by the menu's item: down or up its thread to it, then bobbing
        if (spider_.x < 0) spider_ = ImVec2(at.x, -scale * 40);
        spider_ = lerp(spider_, at, std::min(1.0f, dt * 3.5f));
        draw_spider(list, spider_.x, spider_.y + scale * 3 * std::sin(now * 2.2f), scale, now);
        return;
    }
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
    draw_spider(list, screen.x * 0.34f + std::sin(now * 0.5f) * scale * 14, y, scale, now);
}

// The spider at (x, y), the top of its head, on its thread from the top of the screen.
void WebEffects::draw_spider(ImDrawList* list, float x, float y, float scale, float now) {
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

}  // namespace xport::pc
