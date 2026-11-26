#include "pc/hle_patches.h"

#include <windows.h>

#include <cstdint>
#include <algorithm>
#include <atomic>
#include <cstring>
#include <vector>

#include "pc/exec_memory.h"
#include "pc/game_pause.h"
#include "pc/log.h"
#include "pc/nv2a.h"
#include "pc/renderer_d3d11.h"
#include "pc/settings.h"

namespace xport::pc {
namespace {

// --- time stamp counter ---
//
// The game turns the CPU time stamp counter into time assuming the Xbox's 733 MHz: its clock
// divides rdtsc by 733333 and XAPI's QueryPerformanceFrequency returns 733333333. The PC's counter runs at several
// GHz, so everything timed that way went 5-6 times too fast (menu key repeat made the controller "too sensitive").
// These rdtsc sites read a 733 MHz counter made from the game's performance counter instead (game_ticks: from the
// game's start, without the time it spent paused in the in-game menu).

constexpr uint64_t kXboxCpuHz = 733333333;

uint64_t xbox_time_stamp() {
    static const uint64_t frequency = [] {
        LARGE_INTEGER value;
        QueryPerformanceFrequency(&value);
        return static_cast<uint64_t>(value.QuadPart);
    }();
    const auto counter = static_cast<uint64_t>(game_ticks());
    return counter / frequency * kXboxCpuHz + counter % frequency * kXboxCpuHz / frequency;
}

// Replaces "rdtsc; ret": the result in edx:eax, every other register kept.
__declspec(naked) void rdtsc_function() {
    __asm {
        push ecx
        call xbox_time_stamp
        pop ecx
        ret
    }
}

// Replaces "push edx; rdtsc; pop edx; ret": the low half in eax, edx kept.
__declspec(naked) void rdtsc_low_function() {
    __asm {
        push ecx
        push edx
        call xbox_time_stamp
        pop edx
        pop ecx
        ret
    }
}

// XAPI QueryPerformanceCounter: rdtsc into the LARGE_INTEGER, returns TRUE.
uint32_t __stdcall query_performance_counter(uint64_t* counter) {
    *counter = xbox_time_stamp();
    return 1;
}

// The game's own clock (0x34E96A): where it has no fixed frame rate it reads the time stamp counter and divides by the
// Xbox's 733 MHz. It is patched to read the same 733 MHz counter as the rest (the frame rates it runs at, 29.97 or
// 59.94, need none of this). Replaces "rdtsc; mov [esp+0Ch],eax" (6 bytes) by a call to this and a nop.
__declspec(naked) void game_clock_read() {
    __asm {
        push ecx
        call xbox_time_stamp
        pop ecx
        mov dword ptr [esp + 10h], eax  // the return address is on the stack: [esp+0Ch] of the patched code
        ret
    }
}

// --- GPU channel ---
//
// Direct3D's kick-off flushes the write-back cache and writes PUT, and a helper reads GET, thousands of times a second,
// each a register trap. The flush does nothing on a PC (caches are coherent), so it goes, and PUT and GET are reached
// by a call. [device+0x1C20] can also point at a channel in memory: that one is accessed as before.

// Where the two replacements below go back to, in this build (apply_hle_patches sets them).
uint32_t g_kick_off_return = 0, g_read_get_return = 0;

// Replaces the flush and the write of PUT (0x3C4217-0x3C424A): PUT in edx, device in esi; ecx and edx are left as the
// original left them.
__declspec(naked) void kick_off_without_trap() {
    __asm {
        mov ecx, dword ptr [esi + 0x1C20]
        and edx, 0x0FFFFFFF
        cmp ecx, kNv2aUserChannel
        jne in_memory
        push ecx
        push edx
        push edx
        call nv2a_kick_off
        add esp, 4
        pop edx
        pop ecx
        jmp done
    in_memory:
        mov dword ptr [ecx + 0x40], edx
    done:
        jmp dword ptr [g_kick_off_return]
    }
}

// Replaces the read of GET (0x3C4090-0x3C4098): device in ecx, GET wanted in eax; edx is not live.
__declspec(naked) void read_get_without_trap() {
    __asm {
        mov eax, dword ptr [ecx + 0x1C20]
        cmp eax, kNv2aUserChannel
        jne in_memory
        push ecx
        call nv2a_dma_get
        pop ecx
        jmp done
    in_memory:
        mov eax, dword ptr [eax + 0x44]
    done:
        jmp dword ptr [g_read_get_return]
    }
}

struct JumpPatch {
    const char* name;
    uint32_t address;
    std::vector<uint8_t> original;  // checked before patching
    const void* target;
};

// Addresses of the Italian build (build_address moves them to the others; the bytes are checked either way).
const JumpPatch kJumpPatches[] = {
    {"D3D kick-off", 0x003C4217, {0x8B, 0x88, 0x10, 0x04, 0x10}, reinterpret_cast<const void*>(&kick_off_without_trap)},
    {"D3D read of GET", 0x003C4090, {0x8B, 0x81, 0x20, 0x1C, 0x00}, reinterpret_cast<const void*>(&read_get_without_trap)},
    {"XAPI QueryPerformanceCounter", 0x003907CC, {0x8B, 0x4C, 0x24, 0x04, 0x0F},
     reinterpret_cast<const void*>(&query_performance_counter)},
    {"D3D rdtsc", 0x003C72A0, {0x0F, 0x31, 0xC3, 0xCC, 0xCC}, reinterpret_cast<const void*>(&rdtsc_function)},
};

const JumpPatch kCallPatches[] = {
    {"game clock", 0x0034E96C, {0x0F, 0x31, 0x89, 0x44, 0x24, 0x0C}, reinterpret_cast<const void*>(&game_clock_read)},
};

struct Patch {
    const char* name;
    uint32_t address;
    std::vector<uint8_t> original;  // checked before patching
    std::vector<uint8_t> replacement;
};

// XAPI input functions (XPP library, XDK 5849), found from the game's call sequence:
//   XInitDevices(0, 0); while (XGetDeviceEnumerationStatus() == BUSY); XGetDevices(XDEVICE_TYPE_GAMEPAD); ...
// XInitDevices would start the USB host controller driver; instead nothing is enumerated here, and the controllers
// come from the host through the redirected XGetDevices/XInputOpen/XInputGetState (input_hle.cpp).
const Patch kPatches[] = {
    {"XInitDevices", 0x00444657, {0x55, 0x8D, 0x6C, 0x24, 0x90}, {0xC2, 0x08, 0x00}},       // ret 8
    // D3D's LockRect with D3DLOCK_TILED returns the surface through the 0xF0000000 alias of RAM (the tiled view). The port
    // has no such alias, and the pointer it gives for the same surface at 0x80000000 holds the same bytes.
    {"D3D LockRect tiled alias", 0x003C5338, {0x81, 0xC9, 0x00, 0x00, 0x00, 0xF0}, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}},
    // D3D's wait for a fence compares "handed to the GPU minus fence" with "handed minus done" unsigned, the first
    // read once. When another thread hands more work and the GPU passes the first reading meanwhile, "done" is past
    // "handed", the difference wraps and the last loop spins forever: on the Xbox the window is microseconds, with fast
    // frames it froze Midnight Club 3 (same library). Signed: jge and jl instead of jae and jb.
    {"D3D fence wait, first check", 0x003C43EC, {0x3B, 0xCA, 0x0F, 0x83, 0x1F, 0x01, 0x00, 0x00}, {0x3B, 0xCA, 0x0F, 0x8D}},
    {"D3D fence wait, loop", 0x003C450D, {0x3B, 0xC7, 0x72, 0xF6}, {0x3B, 0xC7, 0x7C}},
    // D3D's vertical blank work retires a queued flip only when its count of blanks equals the flip's target (cmp
    // count, target; jne). The count moves on the game's own poll of the interrupt registers as well as on the
    // interrupt, two threads here where the Xbox had one, so a blank can be counted twice and skip the target: the flip
    // then waits for the count to come round. Retire at or past the target (jl: not yet while count < target).
    {"D3D flip retirement at or past the target", 0x003C7414, {0x75, 0x7F, 0xC7, 0x01, 0x00, 0x00}, {0x7C, 0x7F}},
    // The game's loader thread (0x38DC30) sleeps 30 ms after every piece of work it does: one 128 KB read of
    // AMALGA_XB.PAK at start (151 of them, black screen meanwhile) and the city's streaming later. With the game on
    // one core it also waits for the main thread every round, so the start took 7 to 11 s. Sleep(1) instead:
    // "push 1Eh; call Sleep" becomes "push 1".
    {"loader thread sleep", 0x0038DC79, {0x6A, 0x1E, 0xE8}, {0x6A, 0x01}},
    // The objects near the camera each frame go into one of two lists (World 0x4F6F0C and 0x4F6F10, swapped every frame)
    // allocated for 750 entries {object, value}, filled with no check of that size (World 0x234355 and 0x241348) and then
    // sorted. With setting draw_distance above 200% more objects fade in than that: at 250% 796 were counted on entering
    // the city, and the entries past the end overwrote the next block of the game's heap, which crashed it soon after
    // (sm2port issue 9). Both lists hold 4096 entries: the size allocated, the count each is cleared to, and the count
    // the game compares the two lists over.
    {"near objects list size, first", 0x00233DDE, {0x68, 0x74, 0x17, 0x00, 0x00}, {0x68, 0x04, 0x80, 0x00, 0x00}},
    {"near objects list clear, first", 0x00233DF2, {0xBA, 0xEE, 0x02, 0x00, 0x00}, {0xBA, 0x00, 0x10, 0x00, 0x00}},
    {"near objects list size, second", 0x00233E15, {0x68, 0x74, 0x17, 0x00, 0x00}, {0x68, 0x04, 0x80, 0x00, 0x00}},
    {"near objects list clear, second", 0x00233E29, {0xBA, 0xEE, 0x02, 0x00, 0x00}, {0xBA, 0x00, 0x10, 0x00, 0x00}},
    {"near objects lists compared", 0x0023410D, {0x81, 0xFF, 0xEE, 0x02, 0x00, 0x00}, {0x81, 0xFF, 0x00, 0x10, 0x00, 0x00}},
};

// The address where the patch's original bytes are in this build; 0 if they are not there.
uint32_t find_address(const Xbe& xbe, uint32_t address, const std::vector<uint8_t>& original) {
    const uint32_t candidate = build_address(xbe, address);
    if (candidate && std::memcmp(reinterpret_cast<const void*>(static_cast<uintptr_t>(candidate)), original.data(),
                                 original.size()) == 0)
        return candidate;
    return 0;
}

// The game's clock: seconds of the day, an int the sky and the scripts' game_clock_timer follow. Its address (in the
// uninitialised data, so there are no bytes to check there) comes from the tick that advances it:
//   mov eax,[clock]; inc eax; xor edx,edx; mov ecx,60; mov [clock],eax
volatile uint32_t* g_clock = nullptr;

// Setting time_of_day writes the fixed hour into the clock every frame. The clock is also the scripts'
// game_clock_timer, which missions time themselves on: when a mission set its own hour (the scripts' two "set the
// time" functions) and the port wrote the fixed hour back, its timer ran out at once, and the final fight with Doc
// Ock went back to the flight to the warehouse (reported on 0.1.6). So once a script sets the time, the clock is the
// game's until a save is loaded (the load puts the clock back from the save: the fixed hour is taken again there) or
// the setting changes. Both script functions are done here, as the game does them, with that mark.
std::atomic<bool> g_clock_left_to_game{false};
// The game's time when the fixed hour took the clock: put back when the setting goes back to the game's clock, so the
// game goes on from its own hour instead of the fixed one.
std::atomic<bool> g_clock_held{false};
uint32_t g_clock_before = 0;
float g_clock_setting = -1;

void leave_clock_to_game(const char* how) {
    if (!g_clock_left_to_game.exchange(true)) {
        log_line("HLE: a script %s the time (%02u:%02u): the clock is the game's until a save is loaded", how,
                 *g_clock / 3600 % 24, *g_clock / 60 % 60);
    }
}

void __cdecl script_set_time(int hour, int minute) {  // World 0x2A8180
    *g_clock = static_cast<uint32_t>((hour * 60 + minute) * 60);
    leave_clock_to_game("set");
}

void __cdecl script_advance_time(int hour, int minute) {  // World 0x2A81C0: forward to hour:minute, within a day
    const uint32_t clock = *g_clock;
    int minutes = hour * 60 - static_cast<int>(clock / 60) % 1440 + minute;
    if (minutes < 0) minutes += 1440;
    *g_clock = clock + static_cast<uint32_t>(minutes * 60);
    leave_clock_to_game("moved");
}

void __cdecl save_loaded_clock(uint32_t value) {
    *g_clock = value;
    g_clock_left_to_game = false;
    g_clock_held = false;  // the save's time is the game's own: taken again from here
}

// World 0x2B8129, in the load of a save: "mov [clock],eax" (5 bytes) becomes a call here.
__declspec(naked) void save_loads_clock() {
    __asm {
        push eax
        push ecx
        push edx
        push eax
        call save_loaded_clock
        add esp, 4
        pop edx
        pop ecx
        pop eax
        ret
    }
}

void patch_clock_writers(const Xbe& xbe) {
    const auto clock = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(g_clock));
    const auto with_clock = [&](std::vector<uint8_t> bytes, size_t at) {
        std::memcpy(&bytes[at], &clock, 4);
        return bytes;
    };
    struct Site {
        const char* name;
        uint32_t address;  // Italy
        std::vector<uint8_t> original;
        const void* target;
        uint8_t opcode;  // E9 jmp (a whole function) or E8 call (one instruction)
    };
    const Site sites[] = {
        {"scripts' set time", 0x002A7E80,
         with_clock({0x8B, 0x44, 0x24, 0x04, 0x8B, 0x4C, 0x24, 0x08, 0x6B, 0xC0, 0x3C, 0x03, 0xC1, 0x6B, 0xC0, 0x3C, 0xA3,
                     0, 0, 0, 0, 0xC3}, 17),
         reinterpret_cast<const void*>(&script_set_time), 0xE9},
        {"scripts' move time", 0x002A7EC0,
         with_clock({0xB8, 0x89, 0x88, 0x88, 0x88, 0xF7, 0x25, 0, 0, 0, 0, 0x8B, 0xC2, 0xC1, 0xE8, 0x05, 0x99, 0xB9, 0xA0,
                     0x05, 0x00, 0x00}, 7),
         reinterpret_cast<const void*>(&script_advance_time), 0xE9},
        {"save's clock", 0x002B7E29, with_clock({0xA3, 0, 0, 0, 0, 0x5E}, 1), reinterpret_cast<const void*>(&save_loads_clock),
         0xE8},
    };
    for (const Site& site : sites) {
        const uint32_t address = find_address(xbe, site.address, site.original);
        if (!address) {
            log_line("HLE: %s at %08X does not match this XBE, not redirected", site.name, site.address);
            continue;
        }
        auto* code = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
        code[0] = site.opcode;
        const uint32_t relative = address_of(site.target) - (address + 5);
        std::memcpy(code + 1, &relative, sizeof(relative));
        log_line("HLE: %s at %08X redirected", site.name, address);
    }
}

volatile uint32_t* find_clock(const Xbe& xbe) {
    const uint32_t address = build_address(xbe, 0x002CE4F8);
    if (!address) return nullptr;
    const auto* code = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(address));
    static const uint8_t kMiddle[] = {0x40, 0x33, 0xD2, 0xB9, 0x3C, 0x00, 0x00, 0x00, 0xA3};
    uint32_t read = 0, written = 0;
    std::memcpy(&read, code + 1, 4);
    std::memcpy(&written, code + 14, 4);
    if (code[0] != 0xA1 || std::memcmp(code + 5, kMiddle, sizeof(kMiddle)) != 0 || read != written) return nullptr;
    return reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(read));
}

// The 3D view's field of view: the game turns its value (1 = 90 degrees, changed by the camera as it goes) into degrees
// with a constant (89.9999) right before setting the perspective, in two places: mulss xmm0,[constant]. Both read this
// copy instead, scaled by setting field_of_view (percent; 100 = like the game).
constexpr uint32_t kFieldOfViewSites[] = {0x00261AAE, 0x0026BEA0};
float g_fov_constant = 0;
volatile float g_fov_degrees = 0;

void scale_field_of_view() {
    if (g_fov_constant) g_fov_degrees = g_fov_constant * std::clamp(setting("field_of_view"), 50.0f, 150.0f) / 100;
}

void patch_field_of_view(const Xbe& xbe) {
    for (const uint32_t site : kFieldOfViewSites) {
        const uint32_t address = build_address(xbe, site);
        auto* code = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
        static const uint8_t kMulss[] = {0xF3, 0x0F, 0x59, 0x05};
        uint32_t constant = 0;
        if (address) std::memcpy(&constant, code + 4, 4);
        const auto* value = reinterpret_cast<const float*>(static_cast<uintptr_t>(constant));
        if (!address || std::memcmp(code, kMulss, sizeof(kMulss)) != 0 || !constant || *value < 89 || *value > 91) {
            log_line("HLE: field of view at %08X does not match this XBE, not patched", site);
            continue;
        }
        g_fov_constant = *value;
        const uint32_t ours = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&g_fov_degrees));
        std::memcpy(code + 4, &ours, 4);
        log_line("HLE: field of view at %08X reads the port's copy", address);
    }
    scale_field_of_view();
}

// Setting widescreen 2: the camera's shape in the game's widescreen mode, 16/9, read once (World 0x344A20) into the
// camera; its perspective keeps the vertical angle, so the screen's wider shape (renderer_aspect) widens the view to
// the sides.
constexpr uint32_t kWideShape = 0x0048D970;  // 16/9: 39 8E E3 3F
bool g_wide_shape = false;

void patch_wide_shape(const Xbe& xbe) {
    const float shape = renderer_aspect();
    if (shape <= 16.0f / 9.0f + 0.01f) return;
    const uint32_t address = find_address(xbe, kWideShape, {0x39, 0x8E, 0xE3, 0x3F});
    if (!address) {
        log_line("HLE: widescreen shape at %08X does not match this XBE, not changed", kWideShape);
        return;
    }
    std::memcpy(reinterpret_cast<void*>(static_cast<uintptr_t>(address)), &shape, sizeof(shape));
    g_wide_shape = true;
    log_line("HLE: widescreen shape at %08X set to %.3f", address, shape);
}

// Setting draw_distance (percent; 100 = like the game), applied at start. Two read-only tables of the game, scaled:
// - Objects (cars, pedestrians, props) fade out by a table of 21 entries {radius, start, end, start², end²,
//   1 / (end² - start²)} (World 0x4BF308): five by the object's size (20 m for the smallest to 280 m), then one for
//   each object class (15 to 500 m, the last "never"). It also sets where traffic and pedestrians appear (World
//   0x19B680, called by the spawners), so they come into view further away rather than popping up closer. The game
//   still shortens small objects' distances when many are around (World 0x226B10), as on the Xbox.
// - City regions are drawn in full detail within sqrt(height² + 250²) m of the camera, at most 800 m (World 0x23711E:
//   250² at 0x474F14, read only by this formula in three places, and 800 at 0x4758C8), the district's lower-detail
//   meshes beyond.
constexpr uint32_t kFadeTable = 0x004BF068;          // World 0x4BF308
constexpr uint32_t kRegionBaseSquare = 0x00474C34;   // World 0x474F14: 250 m, squared
constexpr uint32_t kRegionMaximum = 0x004755E8;      // World 0x4758C8: 800 m

std::vector<uint8_t> float_bytes(std::initializer_list<float> values) {
    std::vector<uint8_t> bytes(values.size() * sizeof(float));
    std::memcpy(bytes.data(), values.begin(), bytes.size());
    return bytes;
}

void patch_draw_distance(const Xbe& xbe) {
    const float s = std::clamp(setting_at_start("draw_distance"), 50.0f, 300.0f) / 100;  // 300: the most tried
    if (s == 1) return;
    if (const uint32_t table = find_address(xbe, kFadeTable, float_bytes({1, 20, 20, 400, 400, 1}))) {
        auto* e = reinterpret_cast<float*>(static_cast<uintptr_t>(table));
        for (int i = 0; i < 21; ++i, e += 6) {
            if (e[2] > 100000) continue;  // "never fades"
            e[1] *= s, e[2] *= s, e[3] *= s * s, e[4] *= s * s;
            if (e[4] > e[3]) e[5] = 1 / (e[4] - e[3]);
        }
        log_line("HLE: object fade distances at %08X times %.2f", table, s);
    } else {
        log_line("HLE: object fade distances at %08X do not match this XBE, not changed", kFadeTable);
    }
    const uint32_t base_square = find_address(xbe, kRegionBaseSquare, float_bytes({250 * 250}));
    const uint32_t maximum = find_address(xbe, kRegionMaximum, float_bytes({800}));
    if (base_square && maximum) {
        *reinterpret_cast<float*>(static_cast<uintptr_t>(base_square)) *= s * s;
        *reinterpret_cast<float*>(static_cast<uintptr_t>(maximum)) *= s;
        log_line("HLE: full-detail region distance at %08X, %08X times %.2f", base_square, maximum, s);
    } else {
        log_line("HLE: full-detail region distance at %08X does not match this XBE, not changed", kRegionBaseSquare);
    }
}

}  // namespace

bool hle_wide_shape() { return g_wide_shape; }

void hle_frame() {
    const float hour = setting("time_of_day");
    if (hour != g_clock_setting) g_clock_setting = hour, g_clock_left_to_game = false;  // a new choice takes it again
    // Whole minutes: the tick makes it hh:mm:01, so the game's "new minute" work does not run on every tick.
    if (g_clock && hour >= 0 && hour < 24 && !g_clock_left_to_game) {
        if (!g_clock_held.exchange(true)) g_clock_before = *g_clock;
        *g_clock = static_cast<uint32_t>(hour * 60) * 60;
    } else if (g_clock_held.exchange(false) && hour < 0) {  // not when a script set the time: that hour stays
        *g_clock = g_clock_before;
    }
    scale_field_of_view();
}

uint32_t build_address(const Xbe& xbe, uint32_t italy_address) {
    switch (xbe.entry_point()) {
        case 0x00390610: return italy_address;         // Italy
        case 0x003905D0: return italy_address - 0x40;  // Europe (Fr,De,Es)
        case 0x00390910:  // World: .text (to 0x3BE59C), then the XDK libraries and .rdata, then .data (from 0x4AF200)
            return italy_address + (italy_address < 0x003BE5C0 ? 0x300 : italy_address < 0x004AF200 ? 0x2E0 : 0x2A0);
        default: return 0;
    }
}

void apply_hle_patches(const Xbe& xbe) {
    // The game's frame rate mode (0x4BF698, never written: 1 = 29.97 fps, a frame every second blank; 0 = 59.94 fps,
    // one every blank). The blanks stay at 60 Hz, so at 0 the game draws and steps 60 times a second; when a frame
    // misses blanks, the game's swap callback counts them and the step grows, as on the Xbox.
    if (setting_at_start("frame_rate") >= 60) {
        if (const uint32_t address = find_address(xbe, 0x004BF698, {1, 0, 0, 0})) {
            *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(address)) = 0;
            log_line("HLE: frame rate mode at %08X set to 59.94 fps", address);
            // 120 fps: the blanks come at 120 Hz (nv2a_vblank_hz) and the mode's rate, from which the game takes its
            // step (blanks since the last frame over the rate), is 119.88: the constant the mode's setter (0x34EA50)
            // reads for mode 0, its only reader.
            const float rate = 59.94f;
            std::vector<uint8_t> original(sizeof(rate));
            std::memcpy(original.data(), &rate, sizeof(rate));
            if (const uint32_t constant = setting_at_start("frame_rate") >= 120 ? find_address(xbe, 0x0048F158, original) : 0) {
                *reinterpret_cast<float*>(static_cast<uintptr_t>(constant)) = rate * 2;
                log_line("HLE: frame rate at %08X set to 119.88 fps", constant);
            } else if (setting_at_start("frame_rate") >= 120) {
                log_line("HLE: frame rate constant not found, the game would run at double speed at 120 fps");
            }
        } else {
            log_line("HLE: frame rate mode not found, the game stays at 29.97 fps");
        }
    }
    g_clock = find_clock(xbe);
    if (g_clock) {
        log_line("HLE: game clock at %08X", static_cast<uint32_t>(reinterpret_cast<uintptr_t>(g_clock)));
        patch_clock_writers(xbe);
    } else {
        log_line("HLE: game clock not found, no fixed time of day");
    }
    patch_field_of_view(xbe);
    patch_wide_shape(xbe);
    patch_draw_distance(xbe);
    g_kick_off_return = build_address(xbe, 0x003C424B);
    g_read_get_return = build_address(xbe, 0x003C4099);
    for (const Patch& patch : kPatches) {
        const uint32_t address = find_address(xbe, patch.address, patch.original);
        if (!address) {
            log_line("HLE: %s at %08X does not match this XBE, not patched", patch.name, patch.address);
            continue;
        }
        std::memcpy(reinterpret_cast<void*>(static_cast<uintptr_t>(address)), patch.replacement.data(),
                    patch.replacement.size());
        log_line("HLE: %s at %08X replaced", patch.name, address);
    }
    for (const JumpPatch& patch : kCallPatches) {
        const uint32_t address = find_address(xbe, patch.address, patch.original);
        if (!address) {
            log_line("HLE: %s at %08X does not match this XBE, not patched", patch.name, patch.address);
            continue;
        }
        auto* code = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
        code[0] = 0xE8;  // call rel32
        const uint32_t relative = address_of(patch.target) - (address + 5);
        std::memcpy(code + 1, &relative, sizeof(relative));
        code[5] = 0x90;
        log_line("HLE: %s at %08X called", patch.name, address);
    }
    for (const JumpPatch& patch : kJumpPatches) {
        const uint32_t address = find_address(xbe, patch.address, patch.original);
        if (!address) {
            log_line("HLE: %s at %08X does not match this XBE, not patched", patch.name, patch.address);
            continue;
        }
        auto* code = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address));
        code[0] = 0xE9;  // jmp rel32
        const uint32_t relative = address_of(patch.target) - (address + 5);
        std::memcpy(code + 1, &relative, sizeof(relative));
        log_line("HLE: %s at %08X redirected", patch.name, address);
    }
}

}  // namespace xport::pc
