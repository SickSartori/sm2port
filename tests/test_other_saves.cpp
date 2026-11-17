// Saves in another data folder: found when they are there, the last played folder chosen, copied without losing the
// saves here, and asked about once.
#include <chrono>
#include <string>

#include "pc/other_saves.h"
#include "test.h"

using namespace xport::pc;

namespace {

const std::filesystem::path kTitle = std::filesystem::path("UDATA") / "4156002b";

void make_save(const std::filesystem::path& data, const char* name, const std::string& content, int hours_ago) {
    const std::filesystem::path folder = data / "xbox_hdd" / kTitle / name;
    std::filesystem::create_directories(folder);
    write_file(folder / "SaveMeta.xbx", std::string("meta"));
    write_file(folder / "game", content);
    const auto when = std::filesystem::file_time_type::clock::now() - std::chrono::hours(hours_ago);
    std::filesystem::last_write_time(folder / "SaveMeta.xbx", when);
    std::filesystem::last_write_time(folder / "game", when);
}

std::string save_content(const std::filesystem::path& data, const char* name) {
    return read_file(data / "xbox_hdd" / kTitle / name / "game");
}

}  // namespace

int main() {
    TemporaryFolder folder("sm2port-test-other-saves");
    const std::filesystem::path here = folder.path / "here", documents = folder.path / "documents",
                                old = folder.path / "old", none = folder.path / "none";
    std::filesystem::create_directories(here / "xbox_hdd" / kTitle);
    write_file(here / "xbox_hdd" / kTitle / "TitleMeta.xbx", std::string("title"));  // a file there is not a save

    // Nothing anywhere, a folder that does not exist, the folder in use itself.
    CHECK(!find_other_saves(here, {documents, none, here, {}}));

    // The folder played last wins; a save of the same name here is counted.
    make_save(old, "AAAA", "old", 48);
    make_save(documents, "AAAA", "documents A", 5);
    make_save(documents, "BBBB", "documents B", 6);
    std::optional<OtherSaves> other = find_other_saves(here, {old, documents});
    CHECK(other && other->folder == documents && other->saves.size() == 2);
    CHECK(other && !other->any_here && other->also_here == 0);
    make_save(here, "AAAA", "here A", 1);
    make_save(here, "CCCC", "here C", 2);
    other = find_other_saves(here, {old, documents});
    CHECK(other && other->any_here && other->also_here == 1);
    CHECK(other && other->newest_here > other->newest);

    // Copied: the other folder's saves are here, the one they replace is in saves_backup, the rest is untouched.
    CHECK(other && use_other_saves(here, *other));
    CHECK(save_content(here, "AAAA") == "documents A");
    CHECK(save_content(here, "BBBB") == "documents B");
    CHECK(save_content(here, "CCCC") == "here C");
    CHECK(read_file(here / "saves_backup" / "1" / kTitle / "AAAA" / "game") == "here A");
    CHECK(save_content(documents, "AAAA") == "documents A");  // left where it was
    CHECK(!std::filesystem::exists(here / "xbox_hdd" / kTitle / "AAAA.copying"));

    // A second time, a second backup folder.
    other = find_other_saves(here, {old});
    CHECK(other && other->folder == old && use_other_saves(here, *other));
    CHECK(save_content(here, "AAAA") == "old");
    CHECK(read_file(here / "saves_backup" / "2" / kTitle / "AAAA" / "game") == "documents A");

    // A save that cannot be copied: nothing changes here.
    OtherSaves missing{documents, {kTitle / "BBBB", kTitle / "ZZZZ"}};
    CHECK(!use_other_saves(here, missing));
    CHECK(save_content(here, "BBBB") == "documents B");
    CHECK(!std::filesystem::exists(here / "saves_backup" / "3"));
    CHECK(!std::filesystem::exists(here / "xbox_hdd" / kTitle / "ZZZZ"));

    // Once answered, not asked again.
    CHECK(find_other_saves(here, {documents}));
    other_saves_answered(here, "kept the saves here");
    CHECK(!find_other_saves(here, {documents}));

    return test_result();
}
