// Mouse look: what goes past full stick deflection is spent later, a reversal drops it, it is capped, and the dead zone is skipped.

#include "pc/mouse_stick.h"
#include "test.h"

using namespace xport::pc;

int main() {
    float carry = 0;
    // A small movement goes straight through.
    CHECK(mouse_to_stick(0, 1000, carry) == 1000 && carry == 0);
    // A swipe of three full deflections: one now, two in the next polls without further movement.
    CHECK(mouse_to_stick(0, 3 * 32767.0f, carry) == 32767);
    CHECK(mouse_to_stick(0, 0, carry) == 32767);
    CHECK(mouse_to_stick(0, 0, carry) == 32767);
    CHECK(mouse_to_stick(0, 0, carry) == 0 && carry == 0);
    // Moving back drops what was waiting.
    mouse_to_stick(0, 5 * 32767.0f, carry);
    CHECK(carry > 0);
    CHECK(mouse_to_stick(0, -1000, carry) == -1000 && carry == 0);
    // A huge swipe waits at most kMouseCarryLimit.
    mouse_to_stick(0, 1000 * 32767.0f, carry);
    CHECK(carry == kMouseCarryLimit);
    // Past the dead zone: a small movement starts at its edge, the direction kept; full tilt and no movement untouched.
    int16_t x = 3277, y = 0;  // a tenth of full tilt
    mouse_skip_dead_zone(x, y, 0.2f);
    CHECK(x > 0.27f * 32767 && x < 0.29f * 32767 && y == 0);
    x = -1000, y = -1000;
    mouse_skip_dead_zone(x, y, 0.2f);
    CHECK(x == y && x < -0.14f * 32767);
    x = 32767, y = 32767;
    mouse_skip_dead_zone(x, y, 0.2f);
    CHECK(x == 32767 && y == 32767);
    x = 0, y = 0;
    mouse_skip_dead_zone(x, y, 0.2f);
    CHECK(x == 0 && y == 0);
    return test_result();
}
