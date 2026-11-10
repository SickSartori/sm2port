// Spider-Man 2's part of the shared renderer (sick-xport's renderer_d3d11.cpp calls these): nothing of its own yet.
#include "pc/renderer_d3d11.h"

namespace xport::pc {

RendererNarrowing renderer_game_narrowing(const SurfaceState&, float, float, float, float, uint32_t) { return {}; }

void renderer_game_clear(const SurfaceState&, uint32_t, int&, int&, int&, int&) {}

bool renderer_game_enlarge(const TextureDescriptor&) { return false; }

bool renderer_game_texture(const TextureDescriptor&, const uint8_t*, uint32_t, TextureImage&) { return false; }

void renderer_game_draw(const SurfaceState&, const DrawState&, float[4]) {}

void renderer_game_frame_shown() {}

}  // namespace xport::pc
