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
    // Past the dead zone, each axis on its own: a small movement starts at its edge, the sign kept; full tilt and no
    // movement untouched.
    CHECK(mouse_skip_dead_zone(3277, 0.4f) > 0.45f * 32767 && mouse_skip_dead_zone(3277, 0.4f) < 0.47f * 32767);
    CHECK(mouse_skip_dead_zone(-3277, 0.4f) == -mouse_skip_dead_zone(3277, 0.4f));
    CHECK(mouse_skip_dead_zone(32767, 0.4f) == 32767 && mouse_skip_dead_zone(-32768, 0.4f) == -32768);
    CHECK(mouse_skip_dead_zone(0, 0.4f) == 0);
    return test_result();
}
