#pragma once

#include <cstdint>

#include "core/xbe.h"

namespace sm2::pc {

// Where an address of the Italian build is in this build's XBE: the Europe (Fr,De,Es) build has everything 0x40
// lower, the World one has the game's code 0x300 higher, the XDK libraries 0x2E0 and .data 0x2A0. 0 for an unknown build.
uint32_t build_address(const Xbe& xbe, uint32_t italy_address);

// Replaces XDK library functions that drive hardware the port does not emulate (the USB host controller behind
// the controllers and memory units) with host implementations. Addresses are the Italian build's, moved with
// build_address; every patch checks the original bytes first and is skipped, with a log line, if they differ.
void apply_hle_patches(const Xbe& xbe);

}  // namespace sm2::pc
