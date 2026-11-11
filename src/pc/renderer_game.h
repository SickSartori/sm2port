#pragma once

namespace xport::pc {

// F7: the HUD hidden or shown again (renderer_game.cpp).
void renderer_game_toggle_hud();

// Before each draw, from nv2a_kelvin.cpp on the GPU thread: whether its first vertex is in screen space (w 1).
void renderer_game_note_screen_space(bool screen_space);

// Whether the game's HUD is on screen (its health ring was drawn in the last few frames).
bool renderer_game_hud_up();

}  // namespace xport::pc
