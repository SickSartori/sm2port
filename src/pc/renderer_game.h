#pragma once

namespace xport::pc {

// F7: the HUD hidden or shown again (renderer_game.cpp).
void renderer_game_toggle_hud();

// Whether the game's HUD is on screen (its health ring was drawn in the last few frames).
bool renderer_game_hud_up();

}  // namespace xport::pc
