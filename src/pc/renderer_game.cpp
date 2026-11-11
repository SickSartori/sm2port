// Spider-Man 2's part of the shared renderer (sick-xport's renderer_d3d11.cpp calls these): the health ring kept round
// in 16:9 (setting hud_aspect), the HUD hidden with F7 and the cutscenes' black bars left out (setting letterbox).
#include "pc/renderer_game.h"

#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <unordered_set>

#include "pc/log.h"
#include "pc/renderer_d3d11.h"
#include "pc/settings.h"

namespace xport::pc {
namespace {

std::atomic<bool> g_hud_hidden{false};
std::atomic<uint64_t> g_frames{0};               // frames shown
std::atomic<uint64_t> g_ring_frame{~0ull >> 1};  // the last one with the health ring
bool g_screen_space = false;                     // renderer_game_note_screen_space, GPU thread
std::atomic<int> g_debug_hud{0};                 // SM2_DEBUG_HUD=<n>: the next n flat draws in the log, from each F7

// The health ring (top left, with the spider-sense meter): pieces of a 256x256 atlas, the only HUD texture of that
// format seen there. The game draws it only while playing, not under its menus nor in its cutscenes.
constexpr uint32_t kRingAtlasFormat = 0x08810E29;

bool ring_on_screen() { return g_frames.load() - g_ring_frame.load() <= 3; }

// Flat draws over the 3D scene on a shown frame (nv2a_kelvin.cpp measures them). Narrowed to nothing (factor 0 at
// column 0: no pixel is covered and the scissor rectangle is empty) when they are to go; towards a side for the HUD.
RendererNarrowing narrowing(const SurfaceState& surface, float left, float right, float top, float bottom) {
    RendererNarrowing n;
    if (left > right || !surface.width || !surface.height) return n;
    const float width = static_cast<float>(surface.width), height = static_cast<float>(surface.height);
    const bool full_width = right - left >= 0.9f * width, full_height = bottom - top >= 0.9f * height;
    // F7: everything flat over the scene but what covers the whole picture (fades, the motion blur, the black bars).
    if (g_hud_hidden && !full_width && !full_height) {
        n.factor = 0;
        return n;
    }
    const bool ring = ring_on_screen();
    // Setting letterbox 0: the cutscenes' bars, full-width strips along the top and bottom edges, are not drawn; the
    // scene is drawn in full behind them. Never while the HUD is up.
    if (setting("letterbox") == 0 && !ring && full_width && !full_height &&
        ((top <= 1 && bottom <= 0.25f * height) || (bottom >= height - 1 && top >= 0.75f * height))) {
        n.factor = 0;
        return n;
    }
    // Setting hud_aspect. In 16:9 the game draws its HUD with the pixels of 4:3, so the ring and the minimap come out a
    // third too wide. While the ring is up, what lies wholly in a corner is narrowed back towards its side: the top
    // ones (the ring, with its grey tab) and the lower right one from 0.67 of the width (the minimap); not the lower
    // left one, as the pieces of one box must share a column and the tutorials' box there reaches past the middle
    // (narrowing its pieces each towards its own side would tear it apart). What counts is the part inside the scissor
    // rectangle: the minimap's map is tiles cut to its window.
    const float narrow = (4.0f / 3.0f) / renderer_aspect();
    if (setting("hud_aspect") == 0 || !ring || narrow >= 0.99f) return n;
    left = std::max(left, static_cast<float>(surface.clip_x));
    right = std::min(right, static_cast<float>(surface.clip_x + surface.clip_width));
    top = std::max(top, static_cast<float>(surface.clip_y));
    bottom = std::min(bottom, static_cast<float>(surface.clip_y + surface.clip_height));
    if (left > right || top > bottom) return n;
    if (bottom <= 0.45f * height && right <= 0.35f * width) {
        n.factor = narrow;
    } else if ((bottom <= 0.45f * height && left >= 0.65f * width) || (top >= 0.45f * height && left >= 0.67f * width)) {
        n.factor = narrow;
        n.anchor = width;
    }
    return n;
}

}  // namespace

void renderer_game_toggle_hud() {
    const bool hidden = !g_hud_hidden.load();
    g_hud_hidden = hidden;
    log_line("renderer: HUD %s (F7)", hidden ? "hidden" : "shown");
    char value[16] = {};
    if (GetEnvironmentVariableA("SM2_DEBUG_HUD", value, sizeof(value))) g_debug_hud = std::atoi(value);
}

void renderer_game_note_screen_space(bool screen_space) { g_screen_space = screen_space; }

bool renderer_game_hud_up() { return ring_on_screen(); }

RendererNarrowing renderer_game_narrowing(const SurfaceState& surface, float left, float right, float top, float bottom,
                                          uint32_t texture) {
    const RendererNarrowing n = narrowing(surface, left, right, top, bottom);
    if (g_debug_hud > 0 && left <= right) {
        --g_debug_hud;
        log_line("hud: %.0f..%.0f x %.0f..%.0f on %08X, texture %08X -> %.3f towards %.0f", left, right, top, bottom,
                 surface.color_address, texture, n.factor, n.anchor);
    }
    return n;
}

void renderer_game_clear(const SurfaceState&, uint32_t, int&, int&, int&, int&) {}

bool renderer_game_enlarge(const TextureDescriptor&) { return false; }

bool renderer_game_texture(const TextureDescriptor&, const uint8_t*, uint32_t, TextureImage&) { return false; }

// Called on the GPU thread only, as renderer_game_narrowing.
void renderer_game_draw(const SurfaceState& surface, const DrawState& state, float alpha_reference[4]) {
    const TextureStage& texture = state.textures[0];
    if (!texture.enabled || !renderer_surface_shown(surface.color_address)) return;
    const bool flat = state.screen_left <= state.screen_right;
    if (flat && texture.descriptor.format == kRingAtlasFormat && state.screen_right <= 0.35f * surface.width &&
        state.screen_bottom <= 0.45f * surface.height) {
        g_ring_frame = g_frames.load();
    }
    // F7 and what the measure misses: the tutorials' box's dark fill is not measured flat, but it is drawn in screen
    // space by its alpha (SRC_ALPHA, INV_SRC_ALPHA) with the texture of the box's flat edges. Such draws fade out with the
    // rest. Only in screen space (or with no depth buffer): that texture is the game's stand-in for untextured draws, the
    // 3D ones included.
    static std::unordered_set<uint32_t> hud_textures;  // of flat draws smaller than the picture
    if (flat && state.screen_right - state.screen_left < 0.9f * surface.width &&
        state.screen_bottom - state.screen_top < 0.9f * surface.height) {
        if (hud_textures.size() > 512) hud_textures.clear();
        hud_textures.insert(texture.descriptor.address);
    } else if (g_hud_hidden && !flat && (g_screen_space || !surface.zeta_address) && state.blend_enable &&
               state.blend_source == 0x302 &&
               state.blend_destination == 0x303 && hud_textures.count(texture.descriptor.address)) {
        alpha_reference[2] = 1;
    }
}

void renderer_game_frame_shown() { ++g_frames; }

}  // namespace xport::pc
