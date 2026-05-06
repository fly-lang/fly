#pragma once

#include <string>
#include <vector>
#include <ostream>

namespace flyp {

enum class ErrorCode {
    Resolution = 1,   // exit 1: E001 conflict, E002 not found
    IO         = 2,   // exit 2: file/network/subprocess errors
    Args       = 3,   // exit 3: invalid CLI arguments
};

struct ErrorLine {
    std::string text;
    bool        indent = false; // two-space indent
};

struct FlypError {
    ErrorCode              code;
    std::string            id;       // "E001", "E002", or ""
    std::string            title;
    std::vector<ErrorLine> body;     // context lines
    std::string            hint;     // optional hint block
};

// Print structured error to stderr and return the process exit code.
int report(const FlypError& err);

// Convenience builders
FlypError make_conflict(
    const std::string& package,
    const std::vector<std::pair<std::string, std::string>>& chain); // {requirer, constraint}

FlypError make_not_found(
    const std::string& package,
    const std::string& ref_kind,
    const std::string& ref_value,
    const std::string& git_url,
    const std::vector<std::string>& available);

FlypError make_io(const std::string& message);
FlypError make_args(const std::string& message);
FlypError make_stale_lock();

} // namespace flyp
