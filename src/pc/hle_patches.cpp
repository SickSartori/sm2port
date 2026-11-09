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
// The game turns the CPU time stamp counter into time assuming the Xbox's 733 MHz: its clock
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

struct JumpPatch {
    const char* name;
    uint32_t address;
    std::vector<uint8_t> original;  // checked before patching
    const void* target;
};

// Addresses of the Italian build (build_address moves them to the others; the bytes are checked either way).
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

// XAPI input functions (XPP library, XDK 5849), found from the game's call sequence:
//   XInitDevices(0, 0); while (XGetDeviceEnumerationStatus() == BUSY); XGetDevices(XDEVICE_TYPE_GAMEPAD); ...
// XInitDevices would start the USB host controller driver; instead nothing is enumerated here, and the controllers
// come from the host through the redirected XGetDevices/XInputOpen/XInputGetState (input_hle.cpp).
const Patch kPatches[] = {
    {"XInitDevices", 0x00444657, {0x55, 0x8D, 0x6C, 0x24, 0x90}, {0xC2, 0x08, 0x00}},       // ret 8
    // D3D's LockRect with D3DLOCK_TILED returns the surface through the 0xF0000000 alias of RAM (the tiled view). The port
    // has no such alias, and the pointer it gives for the same surface at 0x80000000 holds the same bytes.
    {"D3D LockRect tiled alias", 0x003C5338, {0x81, 0xC9, 0x00, 0x00, 0x00, 0xF0}, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}},
};

// The address where the patch's original bytes are in this build; 0 if they are not there.
uint32_t find_address(const Xbe& xbe, uint32_t address, const std::vector<uint8_t>& original) {
    const uint32_t candidate = build_address(xbe, address);
    if (candidate && std::memcmp(reinterpret_cast<const void*>(static_cast<uintptr_t>(candidate)), original.data(),
                                 original.size()) == 0)
        return candidate;
    return 0;
}

}  // namespace

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
        } else {
            log_line("HLE: frame rate mode not found, the game stays at 29.97 fps");
        }
    }
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

}  // namespace sm2::pc
