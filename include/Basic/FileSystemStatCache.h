//===--------------------------------------------------------------------------------------------------------------===//
// include/Basic/FileSystemStatCache.h - Caching for 'stat' calls
//
// Part of the Fly Project https://flylang.org
// Under the Apache License v2.0 see LICENSE for details.
// Thank you to LLVM Project https://llvm.org/
//
//===--------------------------------------------------------------------------------------------------------------===//
//
/// \file
/// Defines the FileSystemStatCache interface.
//
//===--------------------------------------------------------------------------------------------------------------===//

#ifndef LLVM_FLY_BASIC_FILESYSTEMSTATCACHE_H
#define LLVM_FLY_BASIC_FILESYSTEMSTATCACHE_H

#include "Basic/LLVM.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <utility>

namespace fly {

/// Abstract interface for introducing a FileManager cache for 'stat'
/// system calls, which is used by precompiled and pretokenized headers to
/// improve performance.
class FileSystemStatCache {
  virtual void anchor();

public:
  virtual ~FileSystemStatCache() = default;

  /// Get the 'stat' information for the specified path, using the cache
  /// to accelerate it if possible.
  ///
  /// \returns \c true if the path does not exist or \c false if it exists.
  ///
  /// If isFile is true, then this lookup should only return success for files
  /// (not directories).  If it is false this lookup should only return
  /// success for directories (not files).  On a successful file lookup, the
  /// implementation can optionally fill in \p F with a valid \p File object and
  /// the client guarantees that it will close it.
  static std::error_code
  get(StringRef Path, llvm::vfs::Status &Status, bool isFile,
      std::unique_ptr<llvm::vfs::File> *F,
      FileSystemStatCache *Cache, llvm::vfs::FileSystem &FS);

protected:
  // FIXME: The pointer here is a non-owning/optional reference to the
  // unique_ptr. Optional<unique_ptr<vfs::File>&> might be nicer, but
  // Optional needs some work to support references so this isn't possible yet.
  virtual std::error_code getStat(StringRef Path, llvm::vfs::Status &Status,
                                  bool isFile,
                                  std::unique_ptr<llvm::vfs::File> *F,
                                  llvm::vfs::FileSystem &FS) = 0;
};

/// A stat "cache" that can be used by FileManager to keep
/// track of the results of stat() calls that occur throughout the
/// execution of the front end.
class MemorizeStatCalls : public FileSystemStatCache {
public:
  /// The set of stat() calls that have been seen.
  llvm::StringMap<llvm::vfs::Status, llvm::BumpPtrAllocator> StatCalls;

  using iterator =
      llvm::StringMap<llvm::vfs::Status,
                      llvm::BumpPtrAllocator>::const_iterator;

  iterator begin() const { return StatCalls.begin(); }
  iterator end() const { return StatCalls.end(); }

  std::error_code getStat(StringRef Path, llvm::vfs::Status &Status,
                          bool isFile,
                          std::unique_ptr<llvm::vfs::File> *F,
                          llvm::vfs::FileSystem &FS) override;
};

} // namespace fly

#endif // LLVM_FLY_BASIC_FILESYSTEMSTATCACHE_H
