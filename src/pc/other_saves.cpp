#include "pc/other_saves.h"

#include <algorithm>
#include <fstream>
#include <system_error>

namespace xport::pc {
namespace {

constexpr char kAnswerFile[] = "other_saves.txt";

// A title's saves are the folders of UDATA\<title id>; the files beside them are the title's name and picture.
std::vector<std::filesystem::path> saves_of(const std::filesystem::path& data) {
    std::vector<std::filesystem::path> saves;
    std::error_code error;
    const std::filesystem::path udata = data / "xbox_hdd" / "UDATA";
    for (const auto& title : std::filesystem::directory_iterator(udata, error)) {
        std::error_code ignored;
        if (!title.is_directory(ignored)) continue;
        for (const auto& save : std::filesystem::directory_iterator(title.path(), ignored)) {
            if (save.is_directory(ignored)) saves.push_back(std::filesystem::path("UDATA") / title.path().filename() / save.path().filename());
        }
    }
    std::sort(saves.begin(), saves.end());
    return saves;
}

std::filesystem::file_time_type newest_file(const std::filesystem::path& folder) {
    std::filesystem::file_time_type newest = std::filesystem::file_time_type::min();  // no file
    std::error_code error;
    for (const auto& item : std::filesystem::recursive_directory_iterator(folder, error)) {
        std::error_code ignored;
        if (!item.is_regular_file(ignored)) continue;
        const std::filesystem::file_time_type written = item.last_write_time(ignored);
        if (!ignored) newest = std::max(newest, written);
    }
    return newest;
}

std::filesystem::file_time_type newest_of(const std::filesystem::path& data, const std::vector<std::filesystem::path>& saves) {
    std::filesystem::file_time_type newest = std::filesystem::file_time_type::min();  // no file
    for (const std::filesystem::path& save : saves) newest = std::max(newest, newest_file(data / "xbox_hdd" / save));
    return newest;
}

bool same_folder(const std::filesystem::path& a, const std::filesystem::path& b) {
    std::error_code error;
    const bool same = std::filesystem::equivalent(a, b, error);
    return !error && same;
}

}  // namespace

std::optional<OtherSaves> find_other_saves(const std::filesystem::path& data, const std::vector<std::filesystem::path>& others) {
    std::error_code error;
    if (std::filesystem::exists(data / kAnswerFile, error)) return std::nullopt;
    std::optional<OtherSaves> found;
    for (const std::filesystem::path& other : others) {
        if (other.empty() || same_folder(other, data)) continue;
        std::vector<std::filesystem::path> saves = saves_of(other);
        if (saves.empty()) continue;
        const std::filesystem::file_time_type newest = newest_of(other, saves);
        if (found && found->newest >= newest) continue;
        found = OtherSaves{other, std::move(saves), 0, false, newest, newest};
    }
    if (!found) return std::nullopt;
    const std::vector<std::filesystem::path> here = saves_of(data);
    found->any_here = !here.empty();
    found->newest_here = newest_of(data, here);
    for (const std::filesystem::path& save : found->saves) {
        if (std::binary_search(here.begin(), here.end(), save)) ++found->also_here;
    }
    return found;
}

bool use_other_saves(const std::filesystem::path& data, const OtherSaves& other) {
    std::error_code error;
    std::filesystem::path backup;  // chosen at the first save that needs it
    struct Done {
        std::filesystem::path target, kept;  // kept: where the save that was here went, if there was one
    };
    std::vector<Done> done;
    std::vector<std::filesystem::path> backup_folders;  // made for the saves that were here
    const auto undo = [&] {
        std::error_code ignored;
        for (auto item = done.rbegin(); item != done.rend(); ++item) {
            std::filesystem::remove_all(item->target, ignored);
            if (!item->kept.empty()) std::filesystem::rename(item->kept, item->target, ignored);
        }
        // The backup's folders, if they are empty again (remove takes no folder with something in it).
        for (std::filesystem::path empty : backup_folders) {
            for (; empty != data && std::filesystem::remove(empty, ignored); empty = empty.parent_path()) {}
        }
    };
    for (const std::filesystem::path& save : other.saves) {
        const std::filesystem::path source = other.folder / "xbox_hdd" / save, target = data / "xbox_hdd" / save;
        // Copied under another name first: a copy that fails halfway never takes the place of a save.
        std::filesystem::path partial = target;
        partial += ".copying";
        std::filesystem::remove_all(partial, error);
        std::filesystem::create_directories(target.parent_path(), error);
        std::filesystem::copy(source, partial, std::filesystem::copy_options::recursive, error);
        if (error) {
            std::error_code ignored;
            std::filesystem::remove_all(partial, ignored);
            undo();
            return false;
        }
        Done item{target, {}};
        if (std::filesystem::exists(target, error)) {
            if (backup.empty()) {
                for (int number = 1; backup.empty() || std::filesystem::exists(backup, error); ++number) {
                    backup = data / "saves_backup" / std::to_string(number);
                }
            }
            item.kept = backup / save;
            std::filesystem::create_directories(item.kept.parent_path(), error);
            backup_folders.push_back(item.kept.parent_path());
            std::filesystem::rename(target, item.kept, error);
            if (error) {
                std::error_code ignored;
                std::filesystem::remove_all(partial, ignored);
                undo();
                return false;
            }
        }
        std::filesystem::rename(partial, target, error);
        if (error) {
            std::error_code ignored;
            std::filesystem::remove_all(partial, ignored);
            if (!item.kept.empty()) std::filesystem::rename(item.kept, target, ignored);
            undo();
            return false;
        }
        done.push_back(item);
    }
    return true;
}

void other_saves_answered(const std::filesystem::path& data, const std::string& answer) {
    std::ofstream file(data / kAnswerFile, std::ios::trunc);
    file << answer << "\n\nDelete this file to be asked again about the saves in another folder.\n";
}

}  // namespace xport::pc
