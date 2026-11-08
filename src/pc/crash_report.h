#pragma once

// A crash leaves sm2port-crash.dmp next to the log, and the next start would overwrite the log. So the first start after
// a crash puts both in sm2port-crash-report.zip, ready to attach to a report.

#include <filesystem>

namespace sm2::pc {

std::filesystem::path crash_report_path();
bool make_crash_report();   // before the log is opened: true if a report was made
bool crash_report_ready();  // one was made at this start: the launcher offers it

}  // namespace sm2::pc
