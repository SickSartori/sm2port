#pragma once

// Saves left in another data folder. The game keeps its files in one folder: Documents\sm2port, the program's own
// folder for a portable copy or one started by RecompLauncher, %LOCALAPPDATA%\sm2port for versions up to 0.1.1. A
// player who changes the way the game is installed or started gets a new, empty folder, and the saves seem lost. Before
// the game starts, the launcher says where they are and offers to copy them, once: after the answer it does not ask
// again.

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace xport::pc {

struct OtherSaves {
    std::filesystem::path folder;              // the other data folder
    std::vector<std::filesystem::path> saves;  // its saves, relative to xbox_hdd: UDATA/<title id>/<save>
    int also_here = 0;                         // how many of them have a save of the same name here
    bool any_here = false;                     // this folder has saves of its own
    // The last written file of the saves there and here; file_time_type::min() without any.
    std::filesystem::file_time_type newest = std::filesystem::file_time_type::min(), newest_here = newest;
};

// The saves of the other folder that was played last, if the player has not been asked yet. `data` is the folder in
// use; of `others`, the ones that are the same folder or have no saves do not count.
std::optional<OtherSaves> find_other_saves(const std::filesystem::path& data,
                                           const std::vector<std::filesystem::path>& others);

// Copies them here. A save of the same name here is first moved to `data`\saves_backup\<n>, never deleted; the other
// folder is left as it is. False if something could not be copied: what was here is put back.
bool use_other_saves(const std::filesystem::path& data, const OtherSaves& other);

// The player has answered: `answer` is written in other_saves.txt, and find_other_saves finds nothing from then on.
void other_saves_answered(const std::filesystem::path& data, const std::string& answer);

// For this start (runtime.cpp): the other folders the game may have used, and what find_other_saves says of them.
std::optional<OtherSaves> pending_other_saves();

}  // namespace xport::pc
