#include "Error.h"
#include <iostream>
#include <sstream>

namespace flyp {

int report(const FlypError& err) {
    std::ostream& out = std::cerr;

    // Header line
    if (!err.id.empty())
        out << "error[" << err.id << "]: " << err.title << "\n\n";
    else
        out << "error: " << err.title << "\n\n";

    // Body
    for (const auto& line : err.body) {
        if (line.indent)
            out << "  " << line.text << "\n";
        else
            out << line.text << "\n";
    }

    // Hint
    if (!err.hint.empty()) {
        out << "\nhint: " << err.hint << "\n";
    }

    out << "\n";
    return static_cast<int>(err.code);
}

FlypError make_conflict(
    const std::string& package,
    const std::vector<std::pair<std::string, std::string>>& chain)
{
    FlypError e;
    e.code  = ErrorCode::Resolution;
    e.id    = "E001";
    e.title = "dependency conflict — cannot resolve";

    e.body.push_back({"conflict chain:", false});
    for (const auto& [requirer, constraint] : chain) {
        std::string line = "  " + requirer + " → " + package + "  (" + constraint + ")";
        e.body.push_back({line, false});
    }

    e.hint = "run `flyp why " + package + "` to see all constraints";
    return e;
}

FlypError make_not_found(
    const std::string& package,
    const std::string& ref_kind,
    const std::string& ref_value,
    const std::string& git_url,
    const std::vector<std::string>& available)
{
    FlypError e;
    e.code  = ErrorCode::Resolution;
    e.id    = "E002";
    e.title = "package not found";

    e.body.push_back({"package: " + package, false});
    e.body.push_back({"source:  " + git_url, false});
    e.body.push_back({ref_kind + " \"" + ref_value + "\" not found in repository", false});

    if (!available.empty()) {
        std::string avail;
        for (size_t i = 0; i < available.size(); ++i) {
            if (i) avail += ", ";
            avail += available[i];
        }
        e.body.push_back({"available: " + avail, false});

        if (!available.empty()) {
            e.hint = "did you mean " + ref_kind + " = \"" + available.back() + "\"?";
        }
    }
    return e;
}

FlypError make_io(const std::string& message) {
    FlypError e;
    e.code  = ErrorCode::IO;
    e.title = message;
    return e;
}

FlypError make_args(const std::string& message) {
    FlypError e;
    e.code  = ErrorCode::Args;
    e.title = message;
    return e;
}

FlypError make_stale_lock() {
    FlypError e;
    e.code  = ErrorCode::IO;
    e.title = "fly.lock is out of date";
    e.body.push_back({"fly.toml has changed since the lock was generated.", false});
    e.body.push_back({"Run `flyp lock` to regenerate it.", true});
    e.hint  = "use `flyp build --frozen` in CI to catch this early";
    return e;
}

} // namespace flyp
