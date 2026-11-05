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
#include "pc/settings.h"

namespace sm2::pc {
namespace {

// --- time stamp counter ---
//
// The game turns the CPU time stamp counter into time assuming the Xbox's 733 MHz: its millisecond clock (0x1A1C00)
// divides rdtsc by 733333 and XAPI's QueryPerformanceFrequency returns 733333333. The PC's counter runs at several
// GHz, so everything timed that way went 5-6 times too fast (menu key repeat made the controller "too sensitive").
// These rdtsc sites read a 733 MHz counter made from the host's performance counter instead, without the time the
// game spent paused (the in-game menu).

constexpr uint64_t kXboxCpuHz = 733333333;

uint64_t xbox_time_stamp() {
    static const uint64_t frequency = [] {
        LARGE_INTEGER value;
        QueryPerformanceFrequency(&value);
        return static_cast<uint64_t>(value.QuadPart);
    }();
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    const auto counter = static_cast<uint64_t>(value.QuadPart - game_paused_ticks());
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

// XAPI QueryPerformanceCounter (0x32633B): rdtsc into the LARGE_INTEGER, returns TRUE.
uint32_t __stdcall query_performance_counter(uint64_t* counter) {
    *counter = xbox_time_stamp();
    return 1;
}

// The game's own clock (0x34E96A): where it has no fixed frame rate it reads the time stamp counter and divides by the
// Xbox's 733 MHz. It is patched to read the same 733 MHz counter as the rest (the frame rate it runs at, 29.97 or 59.94,
// needs none of this). Replaces "rdtsc; mov [esp+0Ch],eax" (6 bytes) by a call to this and a nop.
__declspec(naked) void game_clock_read() {
    __asm {
        push ecx
        call xbox_time_stamp
        pop ecx
        mov dword ptr [esp + 10h], eax  // the return address is on the stack: [esp+0Ch] of the patched code
        ret
    }
}

struct JumpPatch {
    const char* name;
    uint32_t address;
    std::vector<uint8_t> original;  // checked before patching
    const void* target;
};

// Addresses of the Italian build; the Europe (Fr,De,Es) one has them 0x40 lower (the bytes are checked either way).
const JumpPatch kJumpPatches[] = {
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

// XAPI input functions (XPP library, XDK 5849), found from the game's call sequence at 0x84B40:
//   XInitDevices(0, 0); while (XGetDeviceEnumerationStatus() == BUSY); XGetDevices(XDEVICE_TYPE_GAMEPAD); ...
// XInitDevices would start the USB host controller driver; instead nothing is enumerated here, and the controllers
// come from the host through the redirected XGetDevices/XInputOpen/XInputGetState (input_hle.cpp).
const Patch kPatches[] = {
    {"XInitDevices", 0x00444657, {0x55, 0x8D, 0x6C, 0x24, 0x90}, {0xC2, 0x08, 0x00}},       // ret 8
    // D3D's LockRect with D3DLOCK_TILED returns the surface through the 0xF0000000 alias of RAM (the tiled view). The port
    // has no such alias, and the pointer it gives for the same surface at 0x80000000 holds the same bytes.
    {"D3D LockRect tiled alias", 0x003C5338, {0x81, 0xC9, 0x00, 0x00, 0x00, 0xF0}, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}},
};

constexpr uint32_t kEuropeShift = 0x40;

// The address where the patch's original bytes are: the Italian one, or the Europe one 0x40 lower; 0 if neither.
uint32_t find_address(uint32_t address, const std::vector<uint8_t>& original) {
    for (const uint32_t candidate : {address, address - kEuropeShift}) {
        if (std::memcmp(reinterpret_cast<const void*>(static_cast<uintptr_t>(candidate)), original.data(),
                        original.size()) == 0)
            return candidate;
    }
    return 0;
}

}  // namespace

// On the GPU thread, for each new frame shown (kept for the renderer: Spider-Man 2 runs at the game's own fixed step).
void time_frame_shown(uint32_t /*vblanks*/) {}
bool g_game_loaded = false;  // the XBE is in place: its variables can be read


void apply_hle_patches(const Xbe& /*xbe*/) {
    g_game_loaded = true;
    for (const Patch& patch : kPatches) {
        const uint32_t address = find_address(patch.address, patch.original);
        if (!address) {
            log_line("HLE: %s at %08X does not match this XBE, not patched", patch.name, patch.address);
            continue;
        }
        std::memcpy(reinterpret_cast<void*>(static_cast<uintptr_t>(address)), patch.replacement.data(),
                    patch.replacement.size());
        log_line("HLE: %s at %08X replaced", patch.name, address);
    }
    for (const JumpPatch& patch : kCallPatches) {
        const uint32_t address = find_address(patch.address, patch.original);
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
    // The game's frame rate mode (0x4BF698, never written: 1 = 29.97 fps, 0 = 59.94 fps). At 60 the port shows 120
    // blanks a second (frame_rate, nv2a.cpp), so the game takes the 59.94 step or it would run at double speed.
    if (setting_at_start("frame_rate") >= 60) {
        const std::vector<uint8_t> thirty = {1, 0, 0, 0};
        if (const uint32_t address = find_address(0x004BF698, thirty)) {
            *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(address)) = 0;
            log_line("HLE: frame rate mode at %08X set to 59.94 fps", address);
        } else {
            log_line("HLE: frame rate mode not found, the game stays at 29.97 fps");
        }
    }
    for (const JumpPatch& patch : kJumpPatches) {
        const uint32_t address = find_address(patch.address, patch.original);
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

}  // namespace sm2::pc
