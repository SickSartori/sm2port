#pragma once

// Where sm2port reads and writes. The program may be installed in Program Files, where nothing can be written, so
// its own files (settings, saves, log, shader cache) go to %LOCALAPPDATA%\sm2port. A portable copy keeps them next
// to sm2.exe instead: one with a portable.txt there, or one that already has its sm2port.ini there (older versions).

#include <filesystem>

namespace sm2::pc {

std::filesystem::path program_directory();  // sm2.exe's folder
std::filesystem::path data_directory();     // created if missing
bool portable_mode();

}  // namespace sm2::pc
