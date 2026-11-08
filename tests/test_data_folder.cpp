// The move from the old data folder, when the player asks for it: the saves arrive, nothing is overwritten or lost.

#include "pc/paths.h"
#include "test.h"

using namespace sm2::pc;

int main() {
    TemporaryFolder folder("sm2port_test_data_folder");
    const auto from = folder.path / "old", to = folder.path / "new";
    const std::string save = "xbox_hdd/Partition1/UDATA/4156002B/DF2A56878DAE/Player 1.sav";

    // No old folder: nothing to do, nothing made.
    CHECK(move_data_folder(from, to));
    CHECK(!std::filesystem::exists(to));

    // Everything moves, and the empty old folder goes.
    std::filesystem::create_directories((from / save).parent_path());
    std::filesystem::create_directories(from / "logs");
    write_file(from / save, std::string("the first save"));
    write_file(from / "sm2port.ini", std::string("old settings"));
    write_file(from / "logs" / "sm2port.log", std::string("a log"));
    CHECK(move_data_folder(from, to));
    CHECK(read_file(to / save) == "the first save");
    CHECK(read_file(to / "sm2port.ini") == "old settings");
    CHECK(read_file(to / "logs" / "sm2port.log") == "a log");
    CHECK(!std::filesystem::exists(from));

    // An older version ran again afterwards: what exists in the new folder is never overwritten, and the old stays.
    std::filesystem::create_directories((from / save).parent_path());
    write_file(from / save, std::string("an older save"));
    write_file(from / "sm2port.ini", std::string("older settings"));
    write_file(from / "shaders.bin", std::string("new here"));
    CHECK(move_data_folder(from, to));
    CHECK(read_file(to / save) == "the first save");
    CHECK(read_file(to / "sm2port.ini") == "old settings");
    CHECK(read_file(to / "shaders.bin") == "new here");
    CHECK(read_file(from / save) == "an older save");
    CHECK(read_file(from / "sm2port.ini") == "older settings");

    // The destination cannot be made (a file in the way): false, and the saves stay where they are.
    const auto blocked = folder.path / "blocked";
    write_file(blocked, std::string("a file"));
    CHECK(!move_data_folder(from, blocked / "sm2port"));
    CHECK(read_file(from / save) == "an older save");

    return test_result();
}
