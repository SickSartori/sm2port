#pragma once
// Mouse look through the right stick: the stick stops at full deflection, so what a fast swipe moves beyond it is
// kept and spent in the next polls; the camera then turns as far as the mouse went, not only as far as one poll of
// full stick allows (issues 3 and 4: a half turn took a swipe across the desk).
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace xport::pc {

// At most about a quarter of a second of full deflection waits, so the camera never runs on long after the mouse
// stopped; a movement the other way drops what was waiting.
constexpr float kMouseCarryLimit = 16 * 32767.0f;

// `movement`: this poll's mouse movement in stick units. `carry`: what earlier polls left over (kept between calls).
inline int16_t mouse_to_stick(int16_t stick, float movement, float& carry) {
    if ((movement > 0 && carry < 0) || (movement < 0 && carry > 0)) carry = 0;
    const float wanted = stick + movement + carry;
    const float given = std::clamp(wanted, -32768.0f, 32767.0f);
    carry = std::clamp(wanted - given, -kMouseCarryLimit, kMouseCarryLimit);
    return static_cast<int16_t>(given);
}

// Past the game's own dead zone: a slow mouse is a small deflection, which the dead zone would swallow and then let go
// all at once; the movement starts at the dead zone's edge instead. `dead_zone`: the fraction of full tilt the game
// ignores on each axis (0.4 on the camera stick: INDIRIZZI-XBE.md, C+0x954 and C+0x984). An axis already at full
// tilt (a fast swipe, the look keys) or at rest is left alone.
inline int16_t mouse_skip_dead_zone(int16_t axis, float dead_zone) {
    const float f = axis / 32767.0f, m = std::fabs(f);
    if (m < 1e-4f || m >= 1) return axis;
    return static_cast<int16_t>(std::copysign(std::min(1.0f, dead_zone + m * (1 - dead_zone)), f) * 32767.0f);
}

}  // namespace xport::pc
