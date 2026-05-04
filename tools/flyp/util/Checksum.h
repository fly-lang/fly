#pragma once

#include <string>
#include <filesystem>

namespace flyp {

// Returns "sha256:<hex64>" for the contents of a file.
std::string sha256_file(const std::filesystem::path& path);

// Returns "sha256:<hex64>" for an in-memory string.
std::string sha256_string(const std::string& data);

// Verify that sha256_file(path) == expected (format "sha256:<hex>").
// Returns true on match.
bool verify_checksum(const std::filesystem::path& path, const std::string& expected);

} // namespace flyp
